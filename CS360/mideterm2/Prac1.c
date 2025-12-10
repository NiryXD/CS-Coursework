/*
 * nums_dist_pipeline.c (renamed Prac1.c in this workspace)
 *
 * This program demonstrates a small data-processing pipeline using a
 * producer thread, a consumer thread, several worker processes, and a
 * final reporter process. It uses several operating-system features:
 *  - POSIX message queues (mq_*) to pass work items from the consumer to
 *    worker processes.
 *  - Named POSIX semaphores (sem_open/sem_wait/sem_post) to protect a
 *    shared memory array where workers append results.
 *  - POSIX shared memory (shm_open, ftruncate) to store a dynamically
 *    sized array of doubles that workers extend concurrently.
 *  - Threads and a ring buffer with semaphores to decouple file I/O
 *    (producer) from sending work to the message queue (consumer).
 *  - fork/exec and pipes to drive an external reporter (here 'cat')
 *    and redirect its standard output/error to files.
 *
 * The comments below explain each step in plain language.
 */

#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <math.h>
#include <mqueue.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Simple error helpers: DIE prints a message and exits; DIEP prints the
 * system error (perror) and exits. These keep the main code easier to read.
 */
#define DIE(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); exit(1); } while(0)
#define DIEP(msg) do { perror(msg); exit(1); } while(0)

/* Messages put on the POSIX message queue. Workers receive these.
 * - value: the numeric payload
 * - quit: 0 = data message, 1 = quit sentinel telling a worker to stop
 */
struct Msg {
    double value;
    int    quit;   // 0=data, 1=quit
};

/* A small in-process ring buffer used by the producer thread and the
 * consumer thread. The producer reads numbers from a file and writes
 * them into this ring; the consumer reads from the ring and sends
 * messages to the OS message queue. The ring decouples file reading
 * from message-queue sending and smooths short-term bursts.
 */
enum { RING_CAP = 64 };

static double ring_buf[RING_CAP];
static int ring_in = 0, ring_out = 0;
static sem_t sem_empty, sem_full; /* sem_empty counts free slots, sem_full counts used slots */
static pthread_mutex_t ring_mu = PTHREAD_MUTEX_INITIALIZER; /* protects ring_in/out */

/* Global state passed around: message-queue descriptor and worker count */
static mqd_t g_mq = (mqd_t)-1;
static size_t g_workers = 0;

/* Names for IPC objects (must start with '/') */
static const char *g_qname = NULL;
static const char *g_semname = NULL;
static const char *g_shmname = NULL;

/* Shared memory layout:
 * [offset 0] uint64_t count   -- number of doubles stored
 * [offset 8] uint64_t cap     -- capacity (how many doubles fit)
 * [offset 16] array of `cap` doubles
 */
#define HDR_COUNT_OFF  ((off_t)0)
#define HDR_CAP_OFF    ((off_t)8)
#define HDR_SIZE       ((off_t)16)

/*
 * xpread/xpwrite: small helpers that call pread/pwrite in a loop to
 * ensure the entire buffer is transferred, handling EINTR and short
 * reads/writes. This makes file I/O robust against signals.
 */
static int xpread(int fd, void *buf, size_t n, off_t off) {
    char *p = (char*)buf; size_t left = n;
    while (left) {
        ssize_t r = pread(fd, p, left, off);
        if (r == -1 && errno == EINTR) continue; /* interrupted, retry */
        if (r <= 0) return -1; /* EOF or error */
        p += r; off += r; left -= (size_t)r;
    }
    return 0;
}
static int xpwrite(int fd, const void *buf, size_t n, off_t off) {
    const char *p = (const char*)buf; size_t left = n;
    while (left) {
        ssize_t w = pwrite(fd, p, left, off);
        if (w == -1 && errno == EINTR) continue;
        if (w <= 0) return -1;
        p += w; off += w; left -= (size_t)w;
    }
    return 0;
}

// --- Ring buffer helpers: simple producer/consumer using semaphores ---
static void ring_put(double x) {
    /* Wait until there's space, lock the ring, write, then signal data */
    sem_wait(&sem_empty);
    pthread_mutex_lock(&ring_mu);
    ring_buf[ring_in] = x;
    ring_in = (ring_in + 1) % RING_CAP;
    pthread_mutex_unlock(&ring_mu);
    sem_post(&sem_full);
}
static double ring_get(void) {
    /* Wait until there's data, lock the ring, read, then signal free slot */
    sem_wait(&sem_full);
    pthread_mutex_lock(&ring_mu);
    double x = ring_buf[ring_out];
    ring_out = (ring_out + 1) % RING_CAP;
    pthread_mutex_unlock(&ring_mu);
    sem_post(&sem_empty);
    return x;
}

// --- File parsing helpers (one double per line) ---
static bool parse_double_line(const char *s, double *out) {
    /* Trim leading whitespace */
    while (*s==' '||*s=='\t'||*s=='\r') s++;
    if (*s=='\0' || *s=='\n') return false; /* empty line */
    char *end = NULL;
    errno = 0;
    double v = strtod(s, &end);
    if (end == s) return false; /* no conversion */
    if (errno == ERANGE) return false; /* out of range */
    /* skip trailing spaces and an optional newline */
    while (*end==' '||*end=='\t'||*end=='\r') end++;
    if (*end=='\n') end++;
    if (*end != '\0') return false; /* garbage after number */
    *out = v;
    return true;
}

/* Producer thread: read numbers from the input file (one double per
 * line), parse them, and push into the in-process ring buffer. When the
 * file ends the producer pushes a special marker (NaN) to signal EOF.
 */
struct ProducerArg { char *filepath; };
static void *producer_thread(void *argp) {
    struct ProducerArg *arg = (struct ProducerArg*)argp;
    FILE *f = fopen(arg->filepath, "r");
    if (!f) DIEP("fopen input file");

    char line[4096];
    while (fgets(line, sizeof line, f)) {
        double v;
        if (parse_double_line(line, &v)) {
            ring_put(v);
        }
    }
    fclose(f);
    /* end-of-stream marker: NaN (not-a-number) */
    ring_put(NAN);
    return NULL;
}

/* Consumer thread: read values from ring and forward them to the
 * system-wide POSIX message queue. After the producer's EOF marker the
 * consumer sends N 'quit' messages so each worker process knows when to
 * stop.
 */
struct ConsumerArg { /* none, uses globals */ };
static void *consumer_thread(void *argp) {
    (void)argp;
    for (;;) {
        double v = ring_get();
        if (isnan(v)) break; /* end-of-stream */
        struct Msg m = { .value = v, .quit = 0 };
        while (mq_send(g_mq, (const char*)&m, sizeof m, 0) == -1) {
            if (errno == EINTR) continue;
            DIEP("mq_send");
        }
    }
    /* send N quit messages (one per worker) */
    struct Msg q = { .value = 0.0, .quit = 1 };
    for (size_t i = 0; i < g_workers; ++i) {
        while (mq_send(g_mq, (const char*)&q, sizeof q, 0) == -1) {
            if (errno == EINTR) continue;
            DIEP("mq_send quit");
        }
    }
    return NULL;
}

/*
 * Worker process: runs in a separate process (created by fork()). It
 * opens the named message queue to receive numbers, opens the named
 * semaphore to coordinate updates, and opens the shared memory file
 * descriptor to append values. Each received value is appended to the
 * shared memory array inside a semaphore-protected critical section.
 *
 * Key point: multiple worker processes can run in parallel. They don't
 * share normal memory; they coordinate via the named semaphore and the
 * shared memory object (shm_open) which both have names visible system-wide.
 */
static void worker_process(const char *qname, const char *semname, const char *shmname) {
    mqd_t q = mq_open(qname, O_RDONLY);
    if (q == (mqd_t)-1) DIEP("worker mq_open");

    sem_t *lock = sem_open(semname, 0);
    if (lock == SEM_FAILED) DIEP("worker sem_open");

    int fd = shm_open(shmname, O_RDWR, 0);
    if (fd == -1) DIEP("worker shm_open");

    /* Receive loop from the message queue */
    struct Msg m;
    for (;;) {
        ssize_t n;
        do {
            n = mq_receive(q, (char*)&m, sizeof m, NULL);
        } while (n == -1 && errno == EINTR);
        if (n == -1) DIEP("worker mq_receive");
        if ((size_t)n != sizeof m) continue; /* ignore malformed */

        if (m.quit) break; /* quit sentinel received */

        /* Critical section: update shared vector safely */
        if (sem_wait(lock) == -1) DIEP("worker sem_wait");
        uint64_t count, cap;
        if (xpread(fd, &count, sizeof count, HDR_COUNT_OFF) == -1) DIEP("worker pread count");
        if (xpread(fd, &cap, sizeof cap, HDR_CAP_OFF) == -1) DIEP("worker pread cap");

        /* If the array is full, grow it (double capacity, or start at 1024) */
        if (count >= cap) {
            uint64_t newcap = cap ? cap*2 : 1024;
            off_t newsize = HDR_SIZE + (off_t)newcap * (off_t)sizeof(double);
            if (ftruncate(fd, newsize) == -1) DIEP("worker ftruncate");
            if (xpwrite(fd, &newcap, sizeof newcap, HDR_CAP_OFF) == -1) DIEP("worker pwrite cap");
            cap = newcap;
        }
        /* Append the new value and update the count */
        off_t off = HDR_SIZE + (off_t)count * (off_t)sizeof(double);
        if (xpwrite(fd, &m.value, sizeof m.value, off) == -1) DIEP("worker pwrite value");
        count++;
        if (xpwrite(fd, &count, sizeof count, HDR_COUNT_OFF) == -1) DIEP("worker pwrite count");

        if (sem_post(lock) == -1) DIEP("worker sem_post");
    }

    close(fd);
    sem_close(lock);
    mq_close(q);
    _exit(0); /* exit child process immediately */
}

// --- Insertion sort (descending) ---
static void insertion_sort_desc(double *a, size_t n) {
    for (size_t i = 1; i < n; ++i) {
        double key = a[i];
        size_t j = i;
        while (j > 0 && a[j-1] < key) {
            a[j] = a[j-1];
            --j;
        }
        a[j] = key;
    }
}

int main(int argc, char *argv[]) {
    /*
     * Expected arguments:
     *   argv[1] = /qname   (POSIX message queue name)
     *   argv[2] = /semname (named semaphore)
     *   argv[3] = /shmname (named shared memory object)
     *   argv[4] = workers  (number of worker processes)
     *   argv[5] = out_file (where reporter's stdout goes)
     *   argv[6] = err_file (where reporter's stderr goes)
     */
    if (argc != 7) {
        fprintf(stderr, "Usage: %s </qname> </semname> </shmname> <workers> <out_file> <err_file>\n", argv[0]);
        return 2;
    }
    g_qname = argv[1];
    g_semname = argv[2];
    g_shmname = argv[3];
    char *endp = NULL;
    long w = strtol(argv[4], &endp, 10);
    if (!endp || *endp!='\0' || w < 1 || w > 64) DIE("Invalid workers '%s'", argv[4]);
    g_workers = (size_t)w;
    const char *out_path = argv[5];
    const char *err_path = argv[6];

    /* IPC object names must be absolute-like and start with '/' */
    if (g_qname[0] != '/' || g_semname[0] != '/' || g_shmname[0] != '/')
        DIE("All IPC names must start with '/'");

    /* Read input filename from stdin (single line). The program expects
     * the caller to provide the path to the input file on stdin.
     */
    char in_path[4096];
    if (!fgets(in_path, sizeof in_path, stdin)) DIE("Failed to read input filename from stdin");
    /* strip trailing whitespace/newline */
    size_t L = strlen(in_path);
    while (L && (in_path[L-1]=='\n' || in_path[L-1]=='\r' || in_path[L-1]==' ' || in_path[L-1]=='\t')) { in_path[--L] = '\0'; }
    if (L == 0) DIE("Empty input filename");

    /* Start clean: remove any existing named objects with the same names
     * so we can create fresh ones. mq_unlink/sem_unlink/shm_unlink remove
     * the name from the system namespace.
     */
    mq_unlink(g_qname);
    sem_unlink(g_semname);
    shm_unlink(g_shmname);

    /* Create POSIX message queue for communicating work items */
    struct mq_attr attr = {0};
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10; /* maximum messages queued */
    attr.mq_msgsize = sizeof(struct Msg); /* each message carries a double+flag */
    g_mq = mq_open(g_qname, O_CREAT|O_EXCL|O_RDWR, 0600, &attr);
    if (g_mq == (mqd_t)-1) DIEP("mq_open create");

    /* Create named semaphore used by workers to protect shared memory updates */
    sem_t *lock = sem_open(g_semname, O_CREAT|O_EXCL, 0600, 1);
    if (lock == SEM_FAILED) DIEP("sem_open create");

    /* Create shared memory object and initialize count/capacity */
    int shmfd = shm_open(g_shmname, O_CREAT|O_EXCL|O_RDWR, 0600);
    if (shmfd == -1) DIEP("shm_open create");
    uint64_t init_count = 0, init_cap = 1024;
    off_t init_size = HDR_SIZE + (off_t)init_cap * (off_t)sizeof(double);
    if (ftruncate(shmfd, init_size) == -1) DIEP("ftruncate shm");
    if (xpwrite(shmfd, &init_count, sizeof init_count, HDR_COUNT_OFF) == -1) DIEP("pwrite count");
    if (xpwrite(shmfd, &init_cap,   sizeof init_cap,   HDR_CAP_OFF)   == -1) DIEP("pwrite cap");

    /* Fork worker processes. Each worker is its own process executing
     * the worker_process function (which opens the same named IPC
     * objects to receive work and append results).
     */
    pid_t *pids = calloc(g_workers, sizeof *pids);
    if (!pids) DIEP("calloc pids");
    for (size_t i = 0; i < g_workers; ++i) {
        pid_t c = fork();
        if (c < 0) DIEP("fork worker");
        if (c == 0) {
            /* In child: close parent's descriptors and run worker loop */
            mq_close(g_mq);
            close(shmfd);
            sem_close(lock);
            worker_process(g_qname, g_semname, g_shmname);
        }
        pids[i] = c;
    }

    /* Initialize semaphores used by the in-process ring buffer */
    if (sem_init(&sem_empty, 0, RING_CAP) == -1) DIEP("sem_init empty");
    if (sem_init(&sem_full,  0, 0)        == -1) DIEP("sem_init full");

    /* Launch the producer and consumer threads that handle file I/O and
     * forwarding to the message queue.
     */
    pthread_t prod, cons;
    struct ProducerArg parg = { .filepath = in_path };
    if (pthread_create(&prod, NULL, producer_thread, &parg) != 0) DIEP("pthread_create producer");
    if (pthread_create(&cons, NULL, consumer_thread, NULL) != 0) DIEP("pthread_create consumer");

    /* Wait for both threads to finish */
    if (pthread_join(prod, NULL) != 0) DIEP("pthread_join producer");
    if (pthread_join(cons, NULL) != 0) DIEP("pthread_join consumer");

    /* Wait for worker processes to finish. We use waitpid in a loop to
     * handle interruptions by signals (EINTR).
     */
    for (size_t i = 0; i < g_workers; ++i) {
        int st;
        while (waitpid(pids[i], &st, 0) == -1) {
            if (errno == EINTR) continue;
            DIEP("waitpid");
        }
        (void)st; /* ignore status details here */
    }
    free(pids);

    /* Read results from shared memory into a local array and sort */
    uint64_t count;
    if (xpread(shmfd, &count, sizeof count, HDR_COUNT_OFF) == -1) DIEP("pread count");
    double *vals = NULL;
    if (count > 0) {
        vals = malloc(sizeof(double) * count);
        if (!vals) DIEP("malloc vals");
        for (uint64_t i = 0; i < count; ++i) {
            off_t off = HDR_SIZE + (off_t)i * (off_t)sizeof(double);
            if (xpread(shmfd, &vals[i], sizeof(double), off) == -1) DIEP("pread value");
        }
        insertion_sort_desc(vals, (size_t)count);
    }

    /* Reporter: we create a pipe and fork a child which runs 'cat'. The
     * parent writes the sorted values into the pipe, the child reads
     * from the pipe as its stdin. We then redirect the child's stdout
     * and stderr to files using dup2. This demonstrates dup2+exec usage.
     *
     * exec-family functions: execlp replaces the current process image
     * with a new program (here 'cat'). It does not create a new
     * process — that's what fork() does. After fork(), the child calls
     * execlp to become 'cat'. dup2 ensures the child's standard streams
     * are connected to the pipe and files before exec.
     */
    int p[2];
    if (pipe(p) == -1) DIEP("pipe");
    pid_t rep = fork();
    if (rep < 0) DIEP("fork reporter");
    if (rep == 0) {
        /* Child reporter: set up stdin/stdout/stderr and exec 'cat' */
        close(p[1]);
        int fd_out = open(out_path, O_CREAT|O_WRONLY|O_TRUNC, 0644);
        if (fd_out < 0) DIEP("open out");
        int fd_err = open(err_path, O_CREAT|O_WRONLY|O_TRUNC, 0644);
        if (fd_err < 0) DIEP("open err");
        if (dup2(p[0], STDIN_FILENO)  < 0) DIEP("dup2 stdin");
        if (dup2(fd_out, STDOUT_FILENO) < 0) DIEP("dup2 stdout");
        if (dup2(fd_err, STDERR_FILENO) < 0) DIEP("dup2 stderr");
        close(p[0]); close(fd_out); close(fd_err);
        execlp("cat", "cat", (char*)NULL); /* replace child with /bin/cat */
        _exit(127); /* if exec fails, exit child with 127 */
    }
    /* Parent: write sorted values to pipe */
    close(p[0]);
    FILE *w = fdopen(p[1], "w");
    if (!w) DIEP("fdopen");
    if (vals) {
        for (uint64_t i = 0; i < count; ++i) {
            if (fprintf(w, "%.2f\n", vals[i]) < 0) DIEP("fprintf");
        }
    }
    fflush(w);
    fclose(w);

    int st;
    while (waitpid(rep, &st, 0) == -1) {
        if (errno == EINTR) continue;
        DIEP("waitpid reporter");
    }
    if (vals) free(vals);

    /* Cleanup: close and unlink all IPC objects */
    mq_close(g_mq);
    sem_close(lock);
    close(shmfd);

    mq_unlink(g_qname);
    sem_unlink(g_semname);
    shm_unlink(g_shmname);

    return 0;
}
