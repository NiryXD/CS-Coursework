/*
 * duosort.c
 *
 * Small utility that demonstrates a common IPC pattern: the parent
 * process streams numeric data (doubles) to a child over a pipe; the
 * child collects the numbers, sorts them (descending), writes them
 * into POSIX shared memory, and exits. The parent waits, maps the
 * shared memory, redirects stdout to a file, and prints the sorted
 * numbers.
 *
 * This file is heavily commented in plain English to explain the
 * following concepts:
 *  - pipes for parent->child streaming
 *  - fork() to create a child process
 *  - sending a sentinel (NaN) to mark end-of-stream
 *  - dynamic array growth with realloc in the child
 *  - creating/resizing shared memory with shm_open + ftruncate
 *  - mapping shared memory with mmap and reading the layout
 *  - redirecting stdout with dup2 so printed output goes to a file
 *
 * The program preserves original behavior — only comments were added.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

static int xreadn(int fd, void *buf, size_t n) {
    char *p = (char*)buf; size_t left = n;
    while (left) {
        ssize_t r = read(fd, p, left);
        if (r < 0 && errno == EINTR) continue;
        if (r <= 0) return -1; // EOF or error
        p += r; left -= (size_t)r;
    }
    return 0;
}
static int xwriten(int fd, const void *buf, size_t n) {
    const char *p = (const char*)buf; size_t left = n;
    while (left) {
        ssize_t w = write(fd, p, left);
        if (w < 0 && errno == EINTR) continue;
        if (w <= 0) return -1;
        p += w; left -= (size_t)w;
    }
    return 0;
}

static bool parse_double_line(const char *s, double *out) {
    while (*s==' '||*s=='\t'||*s=='\r') s++;
    if (*s=='\0' || *s=='\n') return false;
    errno = 0;
    char *end = NULL;
    double v = strtod(s, &end);
    if (end == s || errno == ERANGE) return false;
    while (*end==' '||*end=='\t'||*end=='\r') end++;
    if (*end=='\n') end++;
    if (*end != '\0') return false;
    *out = v; return true;
}

static void insertion_sort_desc(double *a, size_t n) {
    for (size_t i = 1; i < n; ++i) {
        double key = a[i]; size_t j = i;
        while (j > 0 && a[j-1] < key) { a[j] = a[j-1]; --j; }
        a[j] = key;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <out_file> </shm_name>\n", argv[0]);
        return 2;
    }
    const char *out_file = argv[1];
    const char *shm_name = argv[2];
    if (shm_name[0] != '/') { fprintf(stderr, "shm name must start with '/'\n"); return 2; }

    /*
     * Read the input filename from STDIN. This program expects the
     * parent to be told (via a single line on stdin) which file to
     * read numbers from. We trim trailing whitespace/newline.
     */
    char in_path[4096];
    if (!fgets(in_path, sizeof in_path, stdin)) { perror("read stdin"); return 1; }
    size_t L = strlen(in_path);
    while (L && (in_path[L-1]=='\n' || in_path[L-1]=='\r' || in_path[L-1]==' ' || in_path[L-1]=='\t')) in_path[--L] = '\0';
    if (L == 0) { fprintf(stderr, "empty input filename\n"); return 1; }

    /*
     * Open the input file for streaming. The parent will read the
     * file line-by-line, parse doubles, and write them into the
     * write-end of a pipe so the child can consume them as a stream.
     */
    FILE *fin = fopen(in_path, "r");
    if (!fin) { perror("fopen input"); return 1; }

    int pfd[2];
    if (pipe(pfd) == -1) { perror("pipe"); fclose(fin); return 1; }

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); close(pfd[0]); close(pfd[1]); fclose(fin); return 1; }

    if (pid == 0) {
        /* ---- child: collect, sort, publish via shared memory ----
         * Child's responsibility (after fork):
         *  - close the write end of the pipe (we only read)
         *  - read doubles from the pipe until a NaN sentinel
         *  - grow a local dynamic array as needed (realloc)
         *  - sort the array in descending order
         *  - create a POSIX shared memory object, resize it with
         *    ftruncate to hold [uint64_t count][double array]
         *  - mmap the shared memory and write the data there
         *  - exit
         */
        close(pfd[1]);
        /* start with a small capacity and grow using realloc */
        size_t cap = 32, n = 0;
        double *arr = (double*)malloc(cap * sizeof *arr);
        if (!arr) { perror("malloc"); close(pfd[0]); _exit(1); }

        for (;;) {
            double x;
            /* xreadn reads exactly sizeof(double) bytes from the pipe */
            if (xreadn(pfd[0], &x, sizeof x) == -1) { perror("read pipe"); free(arr); close(pfd[0]); _exit(1); }
            /* The parent sends NAN as an agreed-upon sentinel meaning "end" */
            if (isnan(x)) break;
            if (n == cap) {
                cap *= 2;
                double *p = (double*)realloc(arr, cap * sizeof *p);
                if (!p) { perror("realloc"); free(arr); close(pfd[0]); _exit(1); }
                arr = p;
            }
            arr[n++] = x;
        }
        close(pfd[0]);

        /* Sort locally in descending order */
        insertion_sort_desc(arr, n);

        /*
         * Create shared memory and write a simple layout:
         * [uint64_t count][double data...]
         * We unlink first to avoid collisions with leftovers from previous runs.
         */
        shm_unlink(shm_name); // start clean for repeated runs
        int fd = shm_open(shm_name, O_CREAT|O_EXCL|O_RDWR, 0600);
        if (fd == -1) { perror("shm_open"); free(arr); _exit(1); }
        off_t need = (off_t)sizeof(uint64_t) + (off_t)n * (off_t)sizeof(double);
        if (ftruncate(fd, need) == -1) { perror("ftruncate"); close(fd); free(arr); _exit(1); }

        void *mem = mmap(NULL, (size_t)need, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
        if (mem == MAP_FAILED) { perror("mmap"); close(fd); free(arr); _exit(1); }

        uint64_t *count = (uint64_t*)mem;
        *count = (uint64_t)n; /* write the number of elements */
        double *vec = (double*)((char*)mem + sizeof(uint64_t));
        if (n) memcpy(vec, arr, n * sizeof(double));

        munmap(mem, (size_t)need);
        close(fd);
        free(arr);
        _exit(0);
    }

    // ---- parent: stream doubles to child via pipe ----
    close(pfd[0]);
    char line[4096];
    while (fgets(line, sizeof line, fin)) {
        double v;
        if (parse_double_line(line, &v)) {
            if (xwriten(pfd[1], &v, sizeof v) == -1) { perror("write pipe"); close(pfd[1]); fclose(fin); return 1; }
        }
    }
    fclose(fin);
    // send NaN sentinel
    double nanv = NAN;
    if (xwriten(pfd[1], &nanv, sizeof nanv) == -1) { perror("write sentinel"); close(pfd[1]); return 1; }
    close(pfd[1]);

    // wait child
    int st;
    while (waitpid(pid, &st, 0) == -1) { if (errno == EINTR) continue; perror("waitpid"); return 1; }
    if (!WIFEXITED(st) || WEXITSTATUS(st) != 0) { fprintf(stderr, "child failed\n"); return 1; }

    // map shm and read back
    int sfd = shm_open(shm_name, O_RDONLY, 0);
    if (sfd == -1) { perror("shm_open parent"); return 1; }
    struct stat sb;
    if (fstat(sfd, &sb) == -1) { perror("fstat"); close(sfd); return 1; }
    if ((size_t)sb.st_size < sizeof(uint64_t)) { fprintf(stderr, "shm too small\n"); close(sfd); return 1; }

    void *mem = mmap(NULL, (size_t)sb.st_size, PROT_READ, MAP_SHARED, sfd, 0);
    if (mem == MAP_FAILED) { perror("mmap parent"); close(sfd); return 1; }
    uint64_t count = *(uint64_t*)mem;
    double *vec = (double*)((char*)mem + sizeof(uint64_t));
    if ((off_t)(sizeof(uint64_t) + count*sizeof(double)) > sb.st_size) {
        fprintf(stderr, "shm size mismatch\n"); munmap(mem, (size_t)sb.st_size); close(sfd); return 1;
    }

    /*
     * Redirect stdout to the requested output file so the printed
     * sorted numbers go into the file. dup2 replaces the stdout
     * file descriptor with the opened file descriptor. After dup2
     * we can use printf normally and the data will go to out_file.
     */
    int ofd = open(out_file, O_CREAT|O_WRONLY|O_TRUNC, 0644);
    if (ofd < 0) { perror("open out"); munmap(mem, (size_t)sb.st_size); close(sfd); return 1; }
    if (dup2(ofd, STDOUT_FILENO) < 0) { perror("dup2"); close(ofd); munmap(mem, (size_t)sb.st_size); close(sfd); return 1; }
    close(ofd);

    /* Print the sorted values read from shared memory. Because we've
     * redirected stdout to the output file, these printf calls write
     * into that file rather than the terminal.
     */
    for (uint64_t i = 0; i < count; ++i) printf("%.2f\n", vec[i]);

    munmap(mem, (size_t)sb.st_size);
    close(sfd);
    shm_unlink(shm_name);
    return 0;
}
