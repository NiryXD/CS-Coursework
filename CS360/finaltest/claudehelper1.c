#define _XOPEN_SOURCE 700
#include "exam_helpers.h"
#include <stdarg.h>

// ============================================================================
// 1. COMMAND LINE ARGUMENTS IMPLEMENTATION
// ============================================================================

CmdArgs* parse_args(int argc, char *argv[]) {
    CmdArgs *args = safe_malloc(sizeof(CmdArgs));
    args->argc = argc;
    args->argv = argv;
    args->program_name = argv[0];
    return args;
}

void free_args(CmdArgs *args) {
    free(args);
}

char* get_arg(CmdArgs *args, int index) {
    if (index < 0 || index >= args->argc) return NULL;
    return args->argv[index];
}

int get_arg_count(CmdArgs *args) {
    return args->argc;
}

// ============================================================================
// 2. BUFFERED FILE I/O IMPLEMENTATION
// ============================================================================

BufferedFile* open_file_buffered(const char *filename, const char *mode) {
    BufferedFile *bf = safe_malloc(sizeof(BufferedFile));
    bf->fp = fopen(filename, mode);
    if (bf->fp == NULL) {
        free(bf);
        return NULL;
    }
    bf->filename = str_copy(filename);
    bf->mode = str_copy(mode);
    return bf;
}

void close_file_buffered(BufferedFile *bf) {
    if (bf) {
        if (bf->fp) fclose(bf->fp);
        free(bf->filename);
        free(bf->mode);
        free(bf);
    }
}

char* read_line_buffered(BufferedFile *bf, char *buffer, int size) {
    if (!bf || !bf->fp) return NULL;
    return fgets(buffer, size, bf->fp);
}

int write_line_buffered(BufferedFile *bf, const char *line) {
    if (!bf || !bf->fp) return -1;
    return fprintf(bf->fp, "%s\n", line);
}

int read_all_lines(BufferedFile *bf, char ***lines, int *count) {
    if (!bf || !bf->fp) return -1;
    
    int capacity = 10;
    *count = 0;
    *lines = safe_malloc(capacity * sizeof(char*));
    
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), bf->fp)) {
        // Remove newline
        buffer[strcspn(buffer, "\n")] = '\0';
        
        if (*count >= capacity) {
            capacity *= 2;
            *lines = safe_realloc(*lines, capacity * sizeof(char*));
        }
        (*lines)[*count] = str_copy(buffer);
        (*count)++;
    }
    return *count;
}

void free_lines(char **lines, int count) {
    for (int i = 0; i < count; i++) {
        free(lines[i]);
    }
    free(lines);
}

// ============================================================================
// 3. STRING MANIPULATION IMPLEMENTATION
// ============================================================================

char* str_copy(const char *src) {
    if (!src) return NULL;
    char *dest = safe_malloc(strlen(src) + 1);
    strcpy(dest, src);
    return dest;
}

char* str_find(const char *haystack, const char *needle) {
    if (!haystack || !needle) return NULL;
    return strstr(haystack, needle);
}

int str_span_not(const char *str, const char *reject) {
    if (!str || !reject) return 0;
    return strcspn(str, reject);
}

char* str_concat(const char *s1, const char *s2) {
    if (!s1 && !s2) return NULL;
    if (!s1) return str_copy(s2);
    if (!s2) return str_copy(s1);
    
    char *result = safe_malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

char* str_substring(const char *str, int start, int length) {
    if (!str) return NULL;
    int str_len = strlen(str);
    if (start < 0 || start >= str_len) return NULL;
    if (start + length > str_len) length = str_len - start;
    
    char *result = safe_malloc(length + 1);
    strncpy(result, str + start, length);
    result[length] = '\0';
    return result;
}

char* str_replace(const char *str, const char *old_sub, const char *new_sub) {
    if (!str || !old_sub || !new_sub) return NULL;
    
    char *pos = strstr(str, old_sub);
    if (!pos) return str_copy(str);
    
    int old_len = strlen(old_sub);
    int new_len = strlen(new_sub);
    int str_len = strlen(str);
    
    char *result = safe_malloc(str_len - old_len + new_len + 1);
    
    // Copy part before match
    int prefix_len = pos - str;
    strncpy(result, str, prefix_len);
    result[prefix_len] = '\0';
    
    // Append new substring
    strcat(result, new_sub);
    
    // Append part after match
    strcat(result, pos + old_len);
    
    return result;
}

char** str_split(const char *str, const char *delim, int *count) {
    if (!str || !delim) {
        *count = 0;
        return NULL;
    }
    
    char *copy = str_copy(str);
    int capacity = 10;
    char **parts = safe_malloc(capacity * sizeof(char*));
    *count = 0;
    
    char *token = strtok(copy, delim);
    while (token) {
        if (*count >= capacity) {
            capacity *= 2;
            parts = safe_realloc(parts, capacity * sizeof(char*));
        }
        parts[*count] = str_copy(token);
        (*count)++;
        token = strtok(NULL, delim);
    }
    
    free(copy);
    return parts;
}

void free_split(char **parts, int count) {
    free_lines(parts, count);  // Same implementation
}

char* str_trim(const char *str) {
    if (!str) return NULL;
    
    // Skip leading whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n') str++;
    
    if (*str == '\0') return str_copy("");
    
    // Find end
    const char *end = str + strlen(str) - 1;
    while (end > str && (*end == ' ' || *end == '\t' || *end == '\n')) end--;
    
    int len = end - str + 1;
    char *result = safe_malloc(len + 1);
    strncpy(result, str, len);
    result[len] = '\0';
    return result;
}

// ============================================================================
// 4. THREAD POOL IMPLEMENTATION
// ============================================================================

static void* worker_thread(void *arg) {
    ThreadPool *pool = (ThreadPool*)arg;
    
    while (1) {
        pthread_mutex_lock(&pool->queue_mutex);
        
        // Wait for work or shutdown
        while (pool->queue_head == NULL && !pool->shutdown) {
            pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex);
        }
        
        if (pool->shutdown && pool->queue_head == NULL) {
            pthread_mutex_unlock(&pool->queue_mutex);
            break;
        }
        
        // Get work from queue
        WorkItem *item = pool->queue_head;
        pool->queue_head = item->next;
        if (pool->queue_head == NULL) {
            pool->queue_tail = NULL;
        }
        pool->active_workers++;
        
        pthread_mutex_unlock(&pool->queue_mutex);
        
        // Execute work
        item->func(item->arg);
        free(item);
        
        pthread_mutex_lock(&pool->queue_mutex);
        pool->active_workers--;
        pthread_cond_broadcast(&pool->queue_cond);
        pthread_mutex_unlock(&pool->queue_mutex);
    }
    
    return NULL;
}

ThreadPool* threadpool_create(int num_threads) {
    ThreadPool *pool = safe_malloc(sizeof(ThreadPool));
    pool->threads = safe_malloc(num_threads * sizeof(pthread_t));
    pool->thread_count = num_threads;
    pool->queue_head = NULL;
    pool->queue_tail = NULL;
    pool->shutdown = 0;
    pool->active_workers = 0;
    
    pthread_mutex_init(&pool->queue_mutex, NULL);
    pthread_cond_init(&pool->queue_cond, NULL);
    
    for (int i = 0; i < num_threads; i++) {
        pthread_create(&pool->threads[i], NULL, worker_thread, pool);
    }
    
    return pool;
}

void threadpool_push(ThreadPool *pool, WorkFunction func, void *arg) {
    WorkItem *item = safe_malloc(sizeof(WorkItem));
    item->func = func;
    item->arg = arg;
    item->next = NULL;
    
    pthread_mutex_lock(&pool->queue_mutex);
    
    if (pool->queue_tail == NULL) {
        pool->queue_head = item;
        pool->queue_tail = item;
    } else {
        pool->queue_tail->next = item;
        pool->queue_tail = item;
    }
    
    pthread_cond_signal(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);
}

void threadpool_wait(ThreadPool *pool) {
    pthread_mutex_lock(&pool->queue_mutex);
    while (pool->queue_head != NULL || pool->active_workers > 0) {
        pthread_cond_wait(&pool->queue_cond, &pool->queue_mutex);
    }
    pthread_mutex_unlock(&pool->queue_mutex);
}

void threadpool_destroy(ThreadPool *pool) {
    pthread_mutex_lock(&pool->queue_mutex);
    pool->shutdown = 1;
    pthread_cond_broadcast(&pool->queue_cond);
    pthread_mutex_unlock(&pool->queue_mutex);
    
    for (int i = 0; i < pool->thread_count; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    pthread_mutex_destroy(&pool->queue_mutex);
    pthread_cond_destroy(&pool->queue_cond);
    free(pool->threads);
    free(pool);
}

// ============================================================================
// 5. LINE-ORIENTED I/O IMPLEMENTATION
// ============================================================================

int read_ints(int *arr, int max_count) {
    int count = 0;
    while (count < max_count && scanf("%d", &arr[count]) == 1) {
        count++;
    }
    return count;
}

char* read_single_line(char *buffer, int size) {
    if (fgets(buffer, size, stdin) == NULL) return NULL;
    buffer[strcspn(buffer, "\n")] = '\0';
    return buffer;
}

int read_lines_stdin(char ***lines, int *count) {
    int capacity = 10;
    *count = 0;
    *lines = safe_malloc(capacity * sizeof(char*));
    
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), stdin)) {
        buffer[strcspn(buffer, "\n")] = '\0';
        
        if (*count >= capacity) {
            capacity *= 2;
            *lines = safe_realloc(*lines, capacity * sizeof(char*));
        }
        (*lines)[*count] = str_copy(buffer);
        (*count)++;
    }
    return *count;
}

void write_line(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf("\n");
}

int read_triplets(int triplets[][3], int max_count) {
    int count = 0;
    while (count < max_count && 
           scanf("%d %d %d", &triplets[count][0], &triplets[count][1], &triplets[count][2]) == 3) {
        count++;
    }
    return count;
}

// ============================================================================
// 6. DYNAMIC MEMORY HELPERS IMPLEMENTATION
// ============================================================================

DynamicArray* darray_create(size_t element_size, int initial_capacity) {
    DynamicArray *da = safe_malloc(sizeof(DynamicArray));
    da->data = safe_malloc(element_size * initial_capacity);
    da->count = 0;
    da->capacity = initial_capacity;
    da->element_size = element_size;
    return da;
}

void darray_push(DynamicArray *da, void *element) {
    if (da->count >= da->capacity) {
        da->capacity *= 2;
        da->data = safe_realloc(da->data, da->element_size * da->capacity);
    }
    memcpy((char*)da->data + (da->count * da->element_size), element, da->element_size);
    da->count++;
}

void* darray_get(DynamicArray *da, int index) {
    if (index < 0 || index >= da->count) return NULL;
    return (char*)da->data + (index * da->element_size);
}

void darray_free(DynamicArray *da) {
    if (da) {
        free(da->data);
        free(da);
    }
}

void* safe_realloc(void *ptr, size_t size) {
    void *new_ptr = realloc(ptr, size);
    if (new_ptr == NULL && size > 0) {
        fprintf(stderr, "Error: realloc failed\n");
        exit(1);
    }
    return new_ptr;
}

void* safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL && size > 0) {
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
    }
    return ptr;
}

void* safe_calloc(size_t count, size_t size) {
    void *ptr = calloc(count, size);
    if (ptr == NULL && count > 0 && size > 0) {
        fprintf(stderr, "Error: calloc failed\n");
        exit(1);
    }
    return ptr;
}

// ============================================================================
// 7. SIGNAL HANDLING IMPLEMENTATION
// ============================================================================

volatile sig_atomic_t g_shutdown_flag = 0;

static void shutdown_handler(int sig) {
    (void)sig;  // Suppress unused warning
    g_shutdown_flag = 1;
}

void setup_signal_handler(int signal, SignalHandler handler) {
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(signal, &sa, NULL);
}

void setup_graceful_shutdown(void) {
    setup_signal_handler(SIGINT, shutdown_handler);
    setup_signal_handler(SIGTERM, shutdown_handler);
}

int should_shutdown(void) {
    return g_shutdown_flag;
}

void block_all_signals(void) {
    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}

// ============================================================================
// 8. TIME/DATE FUNCTIONS IMPLEMENTATION
// ============================================================================

TimeInfo* get_current_time(void) {
    TimeInfo *ti = safe_malloc(sizeof(TimeInfo));
    ti->raw_time = time(NULL);
    ti->local_time = localtime(&ti->raw_time);
    strftime(ti->formatted, sizeof(ti->formatted), "%Y-%m-%d %H:%M:%S", ti->local_time);
    return ti;
}

char* format_time(time_t t, const char *format) {
    struct tm *tm_info = localtime(&t);
    char *buffer = safe_malloc(256);
    strftime(buffer, 256, format, tm_info);
    return buffer;
}

char* get_timestamp(void) {
    time_t now = time(NULL);
    return format_time(now, "%Y-%m-%d %H:%M:%S");
}

time_t parse_time(const char *time_str, const char *format) {
    struct tm tm_info = {0};
    strptime(time_str, format, &tm_info);
    return mktime(&tm_info);
}

double elapsed_seconds(time_t start, time_t end) {
    return difftime(end, start);
}

void free_time_info(TimeInfo *ti) {
    free(ti);
}

// ============================================================================
// 9. WORK STRUCTURE IMPLEMENTATION
// ============================================================================

Work* create_work_array(int count) {
    return safe_calloc(count, sizeof(Work));
}

int read_work_stdin(Work **work, int *capacity) {
    int count = 0;
    *capacity = 10;
    *work = safe_malloc(*capacity * sizeof(Work));
    
    while (scanf("%d %d %d", &(*work)[count].param[0], 
                             &(*work)[count].param[1], 
                             &(*work)[count].param[2]) == 3) {
        count++;
        if (count >= *capacity) {
            *capacity *= 2;
            *work = safe_realloc(*work, *capacity * sizeof(Work));
        }
    }
    return count;
}

// ============================================================================
// 10. PTHREAD HELPERS IMPLEMENTATION
// ============================================================================

ThreadArray* thread_array_create(int initial_capacity) {
    ThreadArray *ta = safe_malloc(sizeof(ThreadArray));
    ta->tids = safe_malloc(initial_capacity * sizeof(pthread_t));
    ta->count = 0;
    ta->capacity = initial_capacity;
    return ta;
}

void thread_array_add(ThreadArray *ta, pthread_t tid) {
    if (ta->count >= ta->capacity) {
        ta->capacity *= 2;
        ta->tids = safe_realloc(ta->tids, ta->capacity * sizeof(pthread_t));
    }
    ta->tids[ta->count++] = tid;
}

void thread_array_join_all(ThreadArray *ta) {
    for (int i = 0; i < ta->count; i++) {
        pthread_join(ta->tids[i], NULL);
    }
}

void thread_array_free(ThreadArray *ta) {
    if (ta) {
        free(ta->tids);
        free(ta);
    }
}