// logpool.c
// Multi-threaded log filter using a thread pool

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>

#define MAX_LINE_LEN 1024

/* ---------------------- Signal handling ---------------------- */

static volatile sig_atomic_t got_sigint = 0;

static void handle_sigint(int signo)
{
    (void)signo; /* unused */
    got_sigint = 1;
}

static void install_signal_handler(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; /* do not use SA_RESTART so fgets can be interrupted */

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
}

/* ---------------------- Time helpers ---------------------- */

static pthread_mutex_t time_mtx = PTHREAD_MUTEX_INITIALIZER;

static void format_time(time_t t, char *buf, size_t buf_size)
{
    struct tm *tm_p;

    pthread_mutex_lock(&time_mtx);
    tm_p = localtime(&t); /* not thread-safe without mutex */
    if (!tm_p) {
        snprintf(buf, buf_size, "UNKNOWN_TIME");
        pthread_mutex_unlock(&time_mtx);
        return;
    }
    if (strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S", tm_p) == 0) {
        snprintf(buf, buf_size, "TIME_TOO_LONG");
        pthread_mutex_unlock(&time_mtx);
        return;
    }
    pthread_mutex_unlock(&time_mtx);
}

/* Parse a timestamp "YYYY-MM-DD HH:MM:SS" into struct tm and time_t */
static int parse_timestamp(const char *ts, struct tm *out_tm, time_t *out_time)
{
    int year, mon, mday, hour, min, sec;
    if (sscanf(ts, "%4d-%2d-%2d %2d:%2d:%2d", &year, &mon, &mday,
               &hour, &min, &sec) != 6) {
        return -1;
    }
    memset(out_tm, 0, sizeof(*out_tm));
    out_tm->tm_year = year - 1900;
    out_tm->tm_mon  = mon - 1;
    out_tm->tm_mday = mday;
    out_tm->tm_hour = hour;
    out_tm->tm_min  = min;
    out_tm->tm_sec  = sec;
    out_tm->tm_isdst = -1; /* let mktime determine DST */

    if (out_time) {
        time_t t = mktime(out_tm);
        if (t == (time_t)-1) {
            return -1;
        }
        *out_time = t;
    }
    return 0;
}

/* ---------------------- String helpers ---------------------- */

/* Delete a segment [pos, pos+len) from s, shifting the rest left. */
static void delete_segment(char *s, size_t pos, size_t len)
{
    size_t n = strlen(s);
    if (pos >= n) return;
    if (pos + len > n) {
        len = n - pos;
    }
    /* memmove handles overlap */
    memmove(s + pos, s + pos + len, n - (pos + len) + 1); /* include '\0' */
}

/* Remove trailing newline via delete_segment (demonstrates cut/move). */
static void remove_trailing_newline(char *s)
{
    size_t len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        delete_segment(s, len - 1, 1);
    }
}

/* ---------------------- Thread pool types ---------------------- */

struct shared_context; /* forward */

typedef struct work_item {
    char *line;                 /* heap copy of the input line */
    struct shared_context *ctx; /* shared data */
} work_item_t;

typedef struct task {
    void (*func)(void *arg);
    void *arg;
    struct task *next;
} task_t;

typedef struct thread_pool {
    pthread_t *threads;
    int nthreads;

    task_t *head;
    task_t *tail;
    int stop;

    pthread_mutex_t mtx;
    pthread_cond_t  cond;
} thread_pool_t;

/* ---------------------- Shared context ---------------------- */

typedef struct shared_context {
    FILE *out_fp;
    char *keyword;              /* heap copy of keyword */

    pthread_mutex_t out_mtx;    /* protect output file */
    pthread_mutex_t stats_mtx;  /* protect statistics arrays */

    long matched_lines;

    /* for demonstrating calloc/realloc: store lengths of transformed lines */
    int   *line_lengths;
    size_t lengths_capacity;
} shared_context_t;

/* ---------------------- Thread pool implementation ---------------------- */

static void thread_pool_init(thread_pool_t *pool, int nthreads)
{
    pool->nthreads = nthreads;
    pool->head = pool->tail = NULL;
    pool->stop = 0;

    pool->threads = calloc((size_t)nthreads, sizeof(pthread_t));
    if (!pool->threads) {
        perror("calloc threads");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(&pool->mtx, NULL) != 0) {
        perror("pthread_mutex_init");
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&pool->cond, NULL) != 0) {
        perror("pthread_cond_init");
        exit(EXIT_FAILURE);
    }
}

static void thread_pool_submit(thread_pool_t *pool, void (*func)(void *), void *arg)
{
    task_t *task = (task_t *)malloc(sizeof(task_t));
    if (!task) {
        perror("malloc task");
        exit(EXIT_FAILURE);
    }
    task->func = func;
    task->arg  = arg;
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

static task_t *thread_pool_get_task(thread_pool_t *pool)
{
    task_t *task;

    pthread_mutex_lock(&pool->mtx);
    while (!pool->stop && pool->head == NULL) {
        pthread_cond_wait(&pool->cond, &pool->mtx);
    }

    if (pool->head == NULL && pool->stop) {
        pthread_mutex_unlock(&pool->mtx);
        return NULL; /* no more work */
    }

    task = pool->head;
    pool->head = task->next;
    if (pool->head == NULL) {
        pool->tail = NULL;
    }

    pthread_mutex_unlock(&pool->mtx);
    return task;
}

static void thread_pool_stop(thread_pool_t *pool)
{
    pthread_mutex_lock(&pool->mtx);
    pool->stop = 1;
    pthread_cond_broadcast(&pool->cond);
    pthread_mutex_unlock(&pool->mtx);
}

static void *thread_pool_worker(void *arg)
{
    thread_pool_t *pool = (thread_pool_t *)arg;

    for (;;) {
        task_t *task = thread_pool_get_task(pool);
        if (!task) {
            break; /* pool stopped and empty */
        }
        task->func(task->arg);
        free(task);
    }
    return NULL;
}

static void thread_pool_start(thread_pool_t *pool)
{
    for (int i = 0; i < pool->nthreads; i++) {
        int rc = pthread_create(&pool->threads[i], NULL, thread_pool_worker, pool);
        if (rc != 0) {
            fprintf(stderr, "pthread_create: %d\n", rc);
            exit(EXIT_FAILURE);
        }
    }
}

static void thread_pool_destroy(thread_pool_t *pool)
{
    thread_pool_stop(pool);

    for (int i = 0; i < pool->nthreads; i++) {
        int rc = pthread_join(pool->threads[i], NULL);
        if (rc != 0) {
            fprintf(stderr, "pthread_join: %d\n", rc);
        }
    }

    free(pool->threads);
    pool->threads = NULL;

    /* Free any remaining tasks (should normally be none) */
    task_t *t = pool->head;
    while (t) {
        task_t *next = t->next;
        free(t);
        t = next;
    }

    pthread_mutex_destroy(&pool->mtx);
    pthread_cond_destroy(&pool->cond);
}

/* ---------------------- Work processing ---------------------- */

static void process_work(void *arg)
{
    work_item_t *w = (work_item_t *)arg;
    shared_context_t *ctx = w->ctx;
    char *line = w->line;

    /* Expect timestamp at the beginning: 19 chars "YYYY-MM-DD HH:MM:SS" */
    size_t line_len = strlen(line);
    if (line_len < 21) { /* too short to be valid */
        free(line);
        free(w);
        return;
    }

    /* Copy out timestamp (first 19 characters) */
    char timestamp[32];
    strncpy(timestamp, line, 19);
    timestamp[19] = '\0';

    /* Parse timestamp into time_t using mktime (demonstrates it) */
    struct tm log_tm;
    time_t log_time = 0;
    int have_log_time = (parse_timestamp(timestamp, &log_tm, &log_time) == 0);

    /* After timestamp (19 chars), skip whitespace to get LEVEL */
    char *p = line + 19;
    while (*p == ' ' || *p == '\t') {
        p++;
    }

    char *level_start = p;
    size_t level_len = strcspn(level_start, " \t\r\n");
    if (level_len == 0) {
        free(line);
        free(w);
        return;
    }

    char level[32];
    if (level_len >= sizeof(level)) {
        level_len = sizeof(level) - 1;
    }
    strncpy(level, level_start, level_len);
    level[level_len] = '\0';

    /* Skip level and following whitespace to get message start */
    p = level_start + level_len;
    while (*p == ' ' || *p == '\t') {
        p++;
    }
    char *message_start = p;

    /* Search for keyword in the message using strstr */
    if (strstr(message_start, ctx->keyword) == NULL) {
        free(line);
        free(w);
        return; /* no match */
    }

    /* Build transformed line: "LEVEL YYYY-MM-DD HH:MM:SS: message..." */
    /* Remove trailing newline from original message to avoid duplicates */
    remove_trailing_newline(message_start);

    size_t msg_len = strlen(message_start);
    size_t trans_len = level_len + 1 + 19 + 2 + msg_len + 1; /* +1 for '\0' */

    char *trans = (char *)malloc(trans_len);
    if (!trans) {
        perror("malloc trans");
        free(line);
        free(w);
        return;
    }

    trans[0] = '\0';
    /* Use strcpy/strcat as required */
    strcpy(trans, level);
    strcat(trans, " ");
    strcat(trans, timestamp);
    strcat(trans, ": ");
    strcat(trans, message_start);

    /* Ensure no trailing newline (just in case) by cutting it out */
    remove_trailing_newline(trans);

    /* Get processing time for header */
    time_t now = time(NULL);
    char processed_time[32];
    format_time(now, processed_time, sizeof(processed_time));

    double age_seconds = 0.0;
    if (have_log_time) {
        age_seconds = difftime(now, log_time);
    }

    /* Write to output file with mutex (line-oriented output) */
    pthread_mutex_lock(&ctx->out_mtx);
    fprintf(ctx->out_fp,
            "[PROCESSED AT %s, age=%.2f s] %s\n",
            processed_time,
            age_seconds,
            trans);
    pthread_mutex_unlock(&ctx->out_mtx);

    /* Update statistics: matched_lines and line_lengths (uses realloc) */
    pthread_mutex_lock(&ctx->stats_mtx);

    if ((size_t)ctx->matched_lines >= ctx->lengths_capacity) {
        size_t new_cap = ctx->lengths_capacity ? ctx->lengths_capacity * 2 : 16;
        int *tmp = (int *)realloc(ctx->line_lengths, new_cap * sizeof(int));
        if (!tmp) {
            perror("realloc line_lengths");
            /* don't exit; just skip recording length */
        } else {
            ctx->line_lengths = tmp;
            ctx->lengths_capacity = new_cap;
        }
    }

    if ((size_t)ctx->matched_lines < ctx->lengths_capacity && ctx->line_lengths) {
        ctx->line_lengths[ctx->matched_lines] = (int)strlen(trans);
    }

    ctx->matched_lines++;

    pthread_mutex_unlock(&ctx->stats_mtx);

    /* Clean up work item */
    free(trans);
    free(line);
    free(w);
}

/* ---------------------- Main ---------------------- */

static void usage(const char *prog)
{
    fprintf(stderr, "Usage: %s <num_threads> <input_file> <output_file> <keyword>\n", prog);
}

int main(int argc, char *argv[])
{
    if (argc != 5) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* Handle command line argument: num_threads */
    char *endptr = NULL;
    errno = 0;
    long nthreads_long = strtol(argv[1], &endptr, 10);
    if (errno != 0 || *endptr != '\0' || nthreads_long <= 0) {
        fprintf(stderr, "Invalid num_threads: %s\n", argv[1]);
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    int num_threads = (int)nthreads_long;

    const char *input_path = argv[2];
    const char *output_path = argv[3];
    const char *keyword_arg = argv[4];

    if (keyword_arg[0] == '\0') {
        fprintf(stderr, "Keyword must be non-empty.\n");
        return EXIT_FAILURE;
    }

    /* Install SIGINT handler */
    install_signal_handler();

    /* Record start time */
    time_t start_time = time(NULL);
    char start_str[32];
    format_time(start_time, start_str, sizeof(start_str));

    /* Open files using buffered I/O */
    FILE *in_fp = fopen(input_path, "r");
    if (!in_fp) {
        perror(input_path);
        return EXIT_FAILURE;
    }

    FILE *out_fp = fopen(output_path, "w");
    if (!out_fp) {
        perror(output_path);
        fclose(in_fp);
        return EXIT_FAILURE;
    }

    /* Initialize shared context */
    shared_context_t ctx;
    ctx.out_fp = out_fp;
    ctx.keyword = strdup(keyword_arg);
    if (!ctx.keyword) {
        perror("strdup keyword");
        fclose(in_fp);
        fclose(out_fp);
        return EXIT_FAILURE;
    }

    ctx.matched_lines = 0;
    ctx.lengths_capacity = 16;
    ctx.line_lengths = (int *)calloc(ctx.lengths_capacity, sizeof(int));
    if (!ctx.line_lengths) {
        perror("calloc line_lengths");
        free(ctx.keyword);
        fclose(in_fp);
        fclose(out_fp);
        return EXIT_FAILURE;
    }

    if (pthread_mutex_init(&ctx.out_mtx, NULL) != 0) {
        perror("pthread_mutex_init out_mtx");
        free(ctx.line_lengths);
        free(ctx.keyword);
        fclose(in_fp);
        fclose(out_fp);
        return EXIT_FAILURE;
    }
    if (pthread_mutex_init(&ctx.stats_mtx, NULL) != 0) {
        perror("pthread_mutex_init stats_mtx");
        pthread_mutex_destroy(&ctx.out_mtx);
        free(ctx.line_lengths);
        free(ctx.keyword);
        fclose(in_fp);
        fclose(out_fp);
        return EXIT_FAILURE;
    }

    /* Print header to output (line-oriented output) */
    fprintf(out_fp, "# Logpool started at %s\n", start_str);

    /* Initialize and start thread pool */
    thread_pool_t pool;
    thread_pool_init(&pool, num_threads);
    thread_pool_start(&pool);

    /* Marshaller: read lines and submit work items to pool */
    char buf[MAX_LINE_LEN];
    long total_lines = 0;

    while (!got_sigint && fgets(buf, sizeof(buf), in_fp) != NULL) {
        total_lines++;

        size_t len = strlen(buf);
        char *line_copy = (char *)malloc(len + 1);
        if (!line_copy) {
            perror("malloc line_copy");
            break;
        }
        strcpy(line_copy, buf); /* buffered input -> heap string */

        work_item_t *w = (work_item_t *)malloc(sizeof(work_item_t));
        if (!w) {
            perror("malloc work_item");
            free(line_copy);
            break;
        }
        w->line = line_copy;
        w->ctx  = &ctx;

        thread_pool_submit(&pool, process_work, w);
    }

    /* Stop the pool (no more tasks will be submitted) */
    thread_pool_stop(&pool);

    /* Join worker threads and clean up pool */
    thread_pool_destroy(&pool);

    /* Close files */
    fclose(in_fp);
    fflush(out_fp);

    /* Record end time and compute elapsed */
    time_t end_time = time(NULL);
    double elapsed = difftime(end_time, start_time);

    /* Compute average transformed line length if we have any matches */
    double avg_len = 0.0;
    if (ctx.matched_lines > 0 && ctx.line_lengths) {
        long sum = 0;
        for (long i = 0; i < ctx.matched_lines && (size_t)i < ctx.lengths_capacity; i++) {
            sum += ctx.line_lengths[i];
        }
        avg_len = (double)sum / (double)ctx.matched_lines;
    }

    fprintf(stderr,
            "Processed %ld lines, %ld matched \"%s\" in %.2f seconds. Avg transformed length: %.2f\n",
            total_lines,
            ctx.matched_lines,
            ctx.keyword,
            elapsed,
            avg_len);

    /* Clean up shared context */
    pthread_mutex_destroy(&ctx.out_mtx);
    pthread_mutex_destroy(&ctx.stats_mtx);
    free(ctx.line_lengths);
    free(ctx.keyword);

    fclose(out_fp);

    return EXIT_SUCCESS;
}
