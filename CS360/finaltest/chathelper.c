/* =========================
 *  EXAM HELPER FUNCTIONS
 * ========================= */

#ifndef EXAM_HELPERS_H
#define EXAM_HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>

/* =========================================================
 *  1. COMMAND LINE ARGUMENT HELPERS
 * ========================================================= */

static void usage_and_exit(const char *prog, const char *usage)
{
    fprintf(stderr, "Usage: %s %s\n", prog, usage);
    exit(EXIT_FAILURE);
}

/* Ensure argv[index] exists and return it */
static const char *require_arg(int argc, char **argv,
                               int index,
                               const char *usage)
{
    if (argc <= index) {
        usage_and_exit(argv[0], usage);
    }
    return argv[index];
}

/* Convert string to long with error checking */
static long parse_long_or_die(const char *s, const char *name)
{
    char *end;
    errno = 0;
    long val = strtol(s, &end, 10);

    if (errno != 0 || *end != '\0') {
        fprintf(stderr, "Bad %s: '%s'\n", name, s);
        exit(EXIT_FAILURE);
    }
    return val;
}

/* Get argv[index] as a long integer */
static long require_int_arg(int argc, char **argv,
                            int index,
                            const char *name,
                            const char *usage)
{
    const char *arg = require_arg(argc, argv, index, usage);
    return parse_long_or_die(arg, name);
}

/* =========================================================
 *  2. BUFFERED FILE I/O HELPERS
 * ========================================================= */

/* Open file or exit on error */
static FILE *xfopen(const char *path, const char *mode)
{
    FILE *fp = fopen(path, mode);
    if (!fp) {
        perror(path);
        exit(EXIT_FAILURE);
    }
    return fp;
}

/* Read one line (including trailing newline if any).
   Returns 1 on success, 0 on EOF. */
static int read_line(FILE *fp, char *buf, size_t size)
{
    if (fgets(buf, (int)size, fp) == NULL) {
        return 0;  /* EOF or error */
    }
    return 1;
}

/* Write a line and flush if you want safer output */
static void write_line(FILE *fp, const char *line)
{
    if (fputs(line, fp) == EOF) {
        perror("write_line");
        exit(EXIT_FAILURE);
    }
}

/* Simple file copy using buffered streams */
static void copy_file_stream(const char *src, const char *dst)
{
    char buf[4096];
    FILE *in  = xfopen(src, "r");
    FILE *out = xfopen(dst, "w");

    size_t n;
    while ((n = fread(buf, 1, sizeof buf, in)) > 0) {
        if (fwrite(buf, 1, n, out) != n) {
            perror("fwrite");
            exit(EXIT_FAILURE);
        }
    }

    fclose(in);
    fclose(out);
}

/* =========================================================
 *  3. STRING HELPERS (strcpy, strstr, strcspn, strcat)
 * ========================================================= */

/* Safe strcpy that exits if dst is too small */
static void safe_strcpy(char *dst, size_t dst_size, const char *src)
{
    size_t len = strlen(src);
    if (len + 1 > dst_size) {
        fprintf(stderr, "safe_strcpy: buffer too small\n");
        exit(EXIT_FAILURE);
    }
    strcpy(dst, src);
}

/* Safe strcat for two strings */
static void safe_strcat(char *dst, size_t dst_size, const char *src)
{
    size_t need = strlen(dst) + strlen(src) + 1;
    if (need > dst_size) {
        fprintf(stderr, "safe_strcat: buffer too small\n");
        exit(EXIT_FAILURE);
    }
    strcat(dst, src);
}

/* Find substring helper (wraps strstr) */
static char *find_substring(const char *haystack, const char *needle)
{
    return strstr(haystack, needle);
}

/* First index of any char in reject set (wraps strcspn) */
static size_t first_of_any(const char *s, const char *reject)
{
    return strcspn(s, reject);
}

/* Remove trailing newline if present */
static void strip_newline(char *s)
{
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

/* Copy substring: src[start .. start+len-1] into dest */
static void substr_copy(char *dest, size_t dest_size,
                        const char *src,
                        size_t start, size_t len)
{
    size_t src_len = strlen(src);
    if (start > src_len) {
        dest[0] = '\0';
        return;
    }
    if (start + len > src_len) {
        len = src_len - start;
    }
    if (len + 1 > dest_size) {
        fprintf(stderr, "substr_copy: dest too small\n");
        exit(EXIT_FAILURE);
    }
    memcpy(dest, src + start, len);
    dest[len] = '\0';
}

/* Delete [start, start+len) from s, shifting rest left */
static void substr_delete(char *s, size_t start, size_t len)
{
    size_t n = strlen(s);
    if (start >= n) {
        return;
    }
    if (start + len > n) {
        len = n - start;
    }
    memmove(s + start, s + start + len, n - (start + len) + 1);
}

/* Insert "insert" into s at position pos.
   s must have enough space to hold the result. */
static void substr_insert(char *s, size_t s_size,
                          size_t pos, const char *insert)
{
    size_t n = strlen(s);
    size_t m = strlen(insert);

    if (pos > n) pos = n;

    if (n + m + 1 > s_size) {
        fprintf(stderr, "substr_insert: buffer too small\n");
        exit(EXIT_FAILURE);
    }

    memmove(s + pos + m, s + pos, n - pos + 1); /* include '\0' */
    memcpy(s + pos, insert, m);
}

/* =========================================================
 *  4 and 5. THREAD POOL WITH PTHREADS
 *      - create pool
 *      - push work items from marshaller thread
 * ========================================================= */

typedef struct tp_task {
    void (*func)(void *arg);
    void *arg;
    struct tp_task *next;
} tp_task_t;

typedef struct thread_pool {
    pthread_t *threads;
    int nthreads;

    tp_task_t *head;
    tp_task_t *tail;

    int stop;

    pthread_mutex_t mtx;
    pthread_cond_t cond;
} thread_pool_t;

/* Worker thread function */
static void *thread_pool_worker(void *arg)
{
    thread_pool_t *pool = (thread_pool_t *)arg;

    for (;;) {
        pthread_mutex_lock(&pool->mtx);

        while (!pool->stop && pool->head == NULL) {
            pthread_cond_wait(&pool->cond, &pool->mtx);
        }

        if (pool->stop && pool->head == NULL) {
            pthread_mutex_unlock(&pool->mtx);
            break;
        }

        tp_task_t *task = pool->head;
        pool->head = task->next;
        if (pool->head == NULL) {
            pool->tail = NULL;
        }

        pthread_mutex_unlock(&pool->mtx);

        task->func(task->arg);
        free(task);
    }

    return NULL;
}

/* Create a thread pool with nthreads worker threads */
static int thread_pool_init(thread_pool_t *pool, int nthreads)
{
    int i;

    pool->nthreads = nthreads;
    pool->threads = malloc(sizeof(pthread_t) * (size_t)nthreads);
    if (!pool->threads) return -1;

    pool->head = pool->tail = NULL;
    pool->stop = 0;

    pthread_mutex_init(&pool->mtx, NULL);
    pthread_cond_init(&pool->cond, NULL);

    for (i = 0; i < nthreads; i++) {
        if (pthread_create(&pool->threads[i], NULL,
                           thread_pool_worker, pool) != 0) {
            return -1;
        }
    }
    return 0;
}

/* 5. Push work from marshaller or any thread */
static void thread_pool_submit(thread_pool_t *pool,
                               void (*func)(void *),
                               void *arg)
{
    tp_task_t *task = malloc(sizeof(tp_task_t));
    if (!task) {
        perror("malloc task");
        exit(EXIT_FAILURE);
    }
    task->func = func;
    task->arg = arg;
    task->next = NULL;

    pthread_mutex_lock(&pool->mtx);

    if (pool->tail) {
        pool->tail->next = task;
    } else {
        pool->head = task;
    }
    pool->tail = task;

    pthread_cond_signal(&pool->cond);
    pthread_mutex_unlock(&pool->mtx);
}

/* Shut down pool, wait for workers, free memory */
static void thread_pool_destroy(thread_pool_t *pool)
{
    int i;

    pthread_mutex_lock(&pool->mtx);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mtx);

    for (i = 0; i < pool->nthreads; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    free(pool->threads);

    /* Free any remaining tasks */
    tp_task_t *t = pool->head;
    while (t) {
        tp_task_t *next = t->next;
        free(t);
        t = next;
    }

    pthread_mutex_destroy(&pool->mtx);
    pthread_cond_destroy(&pool->cond);
}

#endif  /* EXAM_HELPERS_H */

/* =========================================================
 *  6. ACCEPT WORK FROM A THREAD POOL INSIDE A WORKER THREAD
 *     (simple job queue + worker loop)
 * ========================================================= */

#include <pthread.h>

/* A single job: function + argument */
typedef struct job {
    void (*func)(void *arg);
    void *arg;
    struct job *next;
} job_t;

/* A job queue shared by all worker threads */
typedef struct job_queue {
    job_t *head;
    job_t *tail;
    int stop;                 /* set to 1 to tell workers to exit */

    pthread_mutex_t mtx;
    pthread_cond_t  cond;
} job_queue_t;

/* Initialize queue */
static void jq_init(job_queue_t *q)
{
    q->head = q->tail = NULL;
    q->stop = 0;
    pthread_mutex_init(&q->mtx, NULL);
    pthread_cond_init(&q->cond, NULL);
}

/* Destroy queue and free remaining jobs (if any) */
static void jq_destroy(job_queue_t *q)
{
    job_t *j = q->head;
    while (j) {
        job_t *next = j->next;
        free(j);
        j = next;
    }
    pthread_mutex_destroy(&q->mtx);
    pthread_cond_destroy(&q->cond);
}

/* Submit job from marshaller/main thread */
static void jq_submit(job_queue_t *q, void (*func)(void *), void *arg)
{
    job_t *j = malloc(sizeof *j);
    if (!j) { perror("malloc"); exit(EXIT_FAILURE); }
    j->func = func;
    j->arg  = arg;
    j->next = NULL;

    pthread_mutex_lock(&q->mtx);
    if (q->tail) q->tail->next = j;
    else        q->head = j;
    q->tail = j;
    pthread_cond_signal(&q->cond);  /* wake one worker */
    pthread_mutex_unlock(&q->mtx);
}

/* Ask all workers to stop when queue empties */
static void jq_stop(job_queue_t *q)
{
    pthread_mutex_lock(&q->mtx);
    q->stop = 1;
    pthread_cond_broadcast(&q->cond);   /* wake everyone */
    pthread_mutex_unlock(&q->mtx);
}

/* Internal: worker pulls one job; returns NULL when should exit */
static job_t *jq_get(job_queue_t *q)
{
    job_t *j;

    pthread_mutex_lock(&q->mtx);
    while (!q->stop && q->head == NULL) {
        pthread_cond_wait(&q->cond, &q->mtx);
    }

    if (q->head == NULL && q->stop) {
        pthread_mutex_unlock(&q->mtx);
        return NULL;    /* no more work */
    }

    j = q->head;
    q->head = j->next;
    if (q->head == NULL) q->tail = NULL;

    pthread_mutex_unlock(&q->mtx);
    return j;
}

/* Worker thread function: this is what “accepts work” */
static void *jq_worker_loop(void *arg)
{
    job_queue_t *q = arg;
    for (;;) {
        job_t *j = jq_get(q);
        if (!j) break;          /* queue stopped and empty */
        j->func(j->arg);        /* run job */
        free(j);
    }
    return NULL;
}

/* Convenience: spawn n worker threads on this queue */
static int jq_spawn_workers(job_queue_t *q, pthread_t *tids, int n)
{
    for (int i = 0; i < n; i++) {
        if (pthread_create(&tids[i], NULL, jq_worker_loop, q) != 0) {
            return -1;
        }
    }
    return 0;
}

/* =========================================================
 *  7–8. LINE-ORIENTED INPUT/OUTPUT HELPERS
 * ========================================================= */

#include <stdio.h>

/* Fixed-size line read from FILE* (returns 1 on success, 0 on EOF) */
static int line_read_fixed(FILE *fp, char *buf, size_t size)
{
    if (fgets(buf, (int)size, fp) == NULL) {
        return 0;   /* EOF or error */
    }
    return 1;
}

/* Same but from stdin */
static int line_read_stdin(char *buf, size_t size)
{
    return line_read_fixed(stdin, buf, size);
}

/* Dynamic line read using getline; caller must free().
   Returns NULL on EOF. */
static char *line_read_alloc(FILE *fp)
{
    char   *line = NULL;
    size_t  cap  = 0;
    ssize_t n    = getline(&line, &cap, fp);
    if (n == -1) {
        free(line);
        return NULL;
    }
    return line;
}

/* Write a line to FILE* (no automatic newline) */
static void line_write(FILE *fp, const char *s)
{
    if (fputs(s, fp) == EOF) {
        perror("fputs");
        exit(EXIT_FAILURE);
    }
}

/* Write a line to stdout */
static void line_write_stdout(const char *s)
{
    line_write(stdout, s);
}

/* Write a line plus newline */
static void line_write_ln(FILE *fp, const char *s)
{
    line_write(fp, s);
    line_write(fp, "\n");
}

/* =========================================================
 *  9. HEAP MEMORY HELPERS (malloc/calloc/realloc/free)
 * ========================================================= */

#include <stdlib.h>

/* Always-checking malloc/calloc/realloc */
static void *xmalloc(size_t size)
{
    void *p = malloc(size);
    if (!p) { perror("malloc"); exit(EXIT_FAILURE); }
    return p;
}

static void *xcalloc(size_t n, size_t size)
{
    void *p = calloc(n, size);
    if (!p) { perror("calloc"); exit(EXIT_FAILURE); }
    return p;
}

static void *xrealloc(void *ptr, size_t size)
{
    void *p = realloc(ptr, size);
    if (!p) { perror("realloc"); exit(EXIT_FAILURE); }
    return p;
}

/* Ensure dynamic array has at least min_needed elements.
   Grows capacity (by doubling) if needed and returns new pointer. */
static void *ensure_capacity(void *ptr,
                             size_t elem_size,
                             size_t *capacity,
                             size_t min_needed)
{
    if (*capacity >= min_needed) return ptr;

    size_t new_cap = (*capacity == 0) ? 1 : *capacity;
    while (new_cap < min_needed) {
        new_cap *= 2;
    }
    ptr = xrealloc(ptr, new_cap * elem_size);
    *capacity = new_cap;
    return ptr;
}

/* Simple wrapper for free (for symmetry) */
static void xfree(void *ptr)
{
    free(ptr);
}

/* =========================================================
 *  10. POSIX SIGNAL HELPERS
 * ========================================================= */

#include <signal.h>
#include <string.h>

static volatile sig_atomic_t got_sigint  = 0;
static volatile sig_atomic_t got_sigterm = 0;

/* Example signal handlers */
static void handle_sigint(int sig)
{
    (void)sig;
    got_sigint = 1;    /* set a flag; don't do heavy work here */
}

static void handle_sigterm(int sig)
{
    (void)sig;
    got_sigterm = 1;
}

/* Install a handler using sigaction (better than signal()) */
static void install_signal_handler(int signum, void (*handler)(int))
{
    struct sigaction sa;
    memset(&sa, 0, sizeof sa);
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;   /* or SA_RESTART if you want */

    if (sigaction(signum, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

/* Example helper to install common handlers at program start */
static void setup_default_signals(void)
{
    install_signal_handler(SIGINT,  handle_sigint);
    install_signal_handler(SIGTERM, handle_sigterm);
}


/* =========================================================
 *  11. WALL-TIME HELPERS
 *     (time, localtime, strftime, mktime)
 * ========================================================= */

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

/* Get current time as time_t */
static time_t now_time(void)
{
    time_t t = time(NULL);
    if (t == (time_t)-1) {
        perror("time");
        exit(EXIT_FAILURE);
    }
    return t;
}

/* Format a time_t into "YYYY-MM-DD HH:MM:SS" in local time.
   buf_size should be at least 20. */
static void format_local_time(time_t t, char *buf, size_t buf_size)
{
    struct tm *tm_p = localtime(&t);
    if (!tm_p) {
        perror("localtime");
        exit(EXIT_FAILURE);
    }

    if (strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S", tm_p) == 0) {
        fprintf(stderr, "strftime output truncated\n");
        exit(EXIT_FAILURE);
    }
}

/* Convenience: get current local time as a formatted string.
   buf_size should be at least 20. */
static void now_local_string(char *buf, size_t buf_size)
{
    time_t t = now_time();
    format_local_time(t, buf, buf_size);
}

/* Build a time_t from components (local time).
   Example: make_time_ymd_hms(2024, 12, 7, 15, 30, 0) */
static time_t make_time_ymd_hms(int year, int mon, int mday,
                                int hour, int min, int sec)
{
    struct tm tm_val;

    tm_val.tm_year = year - 1900;  /* years since 1900 */
    tm_val.tm_mon  = mon - 1;      /* 0..11 */
    tm_val.tm_mday = mday;
    tm_val.tm_hour = hour;
    tm_val.tm_min  = min;
    tm_val.tm_sec  = sec;

    tm_val.tm_isdst = -1;          /* let libc figure out DST */

    time_t t = mktime(&tm_val);    /* convert to time_t using local rules */
    if (t == (time_t)-1) {
        perror("mktime");
        exit(EXIT_FAILURE);
    }
    return t;
}

/* Difference in seconds between two times (a - b) */
static double time_diff_seconds(time_t a, time_t b)
{
    return difftime(a, b);
}

/* Print current local time to a stream with a prefix */
static void print_now(FILE *fp, const char *prefix)
{
    char buf[32];
    now_local_string(buf, sizeof buf);
    fprintf(fp, "%s%s\n", prefix, buf);
}

/* =========================================================
 *  12. GENERAL PTHREAD HELPERS
 *     (thread group, mutex, cond var, barrier)
 * ========================================================= */

#include <pthread.h>

/* Wrapper functions that exit on error */

static void x_pthread_create(pthread_t *t,
                             const pthread_attr_t *attr,
                             void *(*start_routine)(void *),
                             void *arg)
{
    int rc = pthread_create(t, attr, start_routine, arg);
    if (rc != 0) {
        fprintf(stderr, "pthread_create: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_pthread_join(pthread_t t, void **retval)
{
    int rc = pthread_join(t, retval);
    if (rc != 0) {
        fprintf(stderr, "pthread_join: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_mutex_init(pthread_mutex_t *m)
{
    int rc = pthread_mutex_init(m, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_mutex_init: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_mutex_lock(pthread_mutex_t *m)
{
    int rc = pthread_mutex_lock(m);
    if (rc != 0) {
        fprintf(stderr, "pthread_mutex_lock: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_mutex_unlock(pthread_mutex_t *m)
{
    int rc = pthread_mutex_unlock(m);
    if (rc != 0) {
        fprintf(stderr, "pthread_mutex_unlock: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_cond_init(pthread_cond_t *c)
{
    int rc = pthread_cond_init(c, NULL);
    if (rc != 0) {
        fprintf(stderr, "pthread_cond_init: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_cond_wait(pthread_cond_t *c, pthread_mutex_t *m)
{
    int rc = pthread_cond_wait(c, m);
    if (rc != 0) {
        fprintf(stderr, "pthread_cond_wait: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_cond_signal(pthread_cond_t *c)
{
    int rc = pthread_cond_signal(c);
    if (rc != 0) {
        fprintf(stderr, "pthread_cond_signal: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

static void x_cond_broadcast(pthread_cond_t *c)
{
    int rc = pthread_cond_broadcast(c);
    if (rc != 0) {
        fprintf(stderr, "pthread_cond_broadcast: %d\n", rc);
        exit(EXIT_FAILURE);
    }
}

/* Simple thread group:
   useful when the exam wants you to create N threads and then join them. */

typedef struct thread_group {
    pthread_t *tids;
    int        count;
} thread_group_t;

/* Start n threads, all running the same function but with different args[i].
   If args is NULL, each thread gets NULL as arg. */
static void tg_start(thread_group_t *tg,
                     int n,
                     void *(*start_routine)(void *),
                     void **args)
{
    tg->count = n;
    tg->tids  = xmalloc((size_t)n * sizeof(pthread_t));

    for (int i = 0; i < n; i++) {
        void *arg = args ? args[i] : NULL;
        x_pthread_create(&tg->tids[i], NULL, start_routine, arg);
    }
}

/* Join all threads and free storage */
static void tg_join_and_destroy(thread_group_t *tg)
{
    for (int i = 0; i < tg->count; i++) {
        x_pthread_join(tg->tids[i], NULL);
    }
    free(tg->tids);
    tg->tids  = NULL;
    tg->count = 0;
}

/* Simple reusable barrier using mutex + cond.
   All threads call barrier_wait; the last thread wakes everyone. */

typedef struct simple_barrier {
    int             count;     /* number of threads required */
    int             waiting;   /* number currently waiting */
    pthread_mutex_t mtx;
    pthread_cond_t  cond;
} simple_barrier_t;

static void barrier_init(simple_barrier_t *b, int count)
{
    b->count   = count;
    b->waiting = 0;
    x_mutex_init(&b->mtx);
    x_cond_init(&b->cond);
}

static void barrier_destroy(simple_barrier_t *b)
{
    pthread_mutex_destroy(&b->mtx);
    pthread_cond_destroy(&b->cond);
}

/* All threads call this. When the last one arrives,
   everyone is released and waiting resets for reuse. */
static void barrier_wait(simple_barrier_t *b)
{
    x_mutex_lock(&b->mtx);
    b->waiting++;

    if (b->waiting == b->count) {
        b->waiting = 0;
        x_cond_broadcast(&b->cond);
    } else {
        while (b->waiting != 0) {
            x_cond_wait(&b->cond, &b->mtx);
        }
    }

    x_mutex_unlock(&b->mtx);
}
