#ifndef EXAM_HELPERS_H
#define EXAM_HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

// ============================================================================
// 1. COMMAND LINE ARGUMENTS
// ============================================================================

typedef struct {
    int argc;
    char **argv;
    char *program_name;
} CmdArgs;

// Parse command line arguments
CmdArgs* parse_args(int argc, char *argv[]);
void free_args(CmdArgs *args);
char* get_arg(CmdArgs *args, int index);
int get_arg_count(CmdArgs *args);

// ============================================================================
// 2. BUFFERED FILE I/O
// ============================================================================

typedef struct {
    FILE *fp;
    char *filename;
    char *mode;
} BufferedFile;

BufferedFile* open_file_buffered(const char *filename, const char *mode);
void close_file_buffered(BufferedFile *bf);
char* read_line_buffered(BufferedFile *bf, char *buffer, int size);
int write_line_buffered(BufferedFile *bf, const char *line);
int read_all_lines(BufferedFile *bf, char ***lines, int *count);
void free_lines(char **lines, int count);

// ============================================================================
// 3. STRING MANIPULATION HELPERS
// ============================================================================

// Copy string safely
char* str_copy(const char *src);

// Find substring and return pointer
char* str_find(const char *haystack, const char *needle);

// Get length until any char in reject set (like strcspn)
int str_span_not(const char *str, const char *reject);

// Concatenate strings (returns new allocated string)
char* str_concat(const char *s1, const char *s2);

// Extract substring (start index, length)
char* str_substring(const char *str, int start, int length);

// Replace first occurrence of old with new
char* str_replace(const char *str, const char *old_sub, const char *new_sub);

// Split string by delimiter
char** str_split(const char *str, const char *delim, int *count);
void free_split(char **parts, int count);

// Trim whitespace from both ends
char* str_trim(const char *str);

// ============================================================================
// 4. THREAD POOL
// ============================================================================

typedef void* (*WorkFunction)(void *arg);

typedef struct WorkItem {
    WorkFunction func;
    void *arg;
    struct WorkItem *next;
} WorkItem;

typedef struct {
    pthread_t *threads;
    int thread_count;
    WorkItem *queue_head;
    WorkItem *queue_tail;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
    int shutdown;
    int active_workers;
} ThreadPool;

// Create thread pool with n worker threads
ThreadPool* threadpool_create(int num_threads);

// Push work to the pool (marshaller calls this)
void threadpool_push(ThreadPool *pool, WorkFunction func, void *arg);

// Wait for all work to complete
void threadpool_wait(ThreadPool *pool);

// Destroy the thread pool
void threadpool_destroy(ThreadPool *pool);

// ============================================================================
// 5. LINE-ORIENTED I/O
// ============================================================================

// Read integers from stdin (returns count of items read)
int read_ints(int *arr, int max_count);

// Read a single line from stdin
char* read_single_line(char *buffer, int size);

// Read lines until EOF
int read_lines_stdin(char ***lines, int *count);

// Write formatted line to stdout
void write_line(const char *format, ...);

// Read 3 integers per line (like your midterm)
int read_triplets(int triplets[][3], int max_count);

// ============================================================================
// 6. DYNAMIC MEMORY HELPERS
// ============================================================================

typedef struct {
    void *data;
    int count;
    int capacity;
    size_t element_size;
} DynamicArray;

// Create dynamic array
DynamicArray* darray_create(size_t element_size, int initial_capacity);

// Add element (auto-resizes)
void darray_push(DynamicArray *da, void *element);

// Get element at index
void* darray_get(DynamicArray *da, int index);

// Free dynamic array
void darray_free(DynamicArray *da);

// Simple wrapper for realloc with error checking
void* safe_realloc(void *ptr, size_t size);

// Simple wrapper for malloc with error checking
void* safe_malloc(size_t size);

// Simple wrapper for calloc with error checking
void* safe_calloc(size_t count, size_t size);

// ============================================================================
// 7. SIGNAL HANDLING
// ============================================================================

typedef void (*SignalHandler)(int);

// Global flag for graceful shutdown
extern volatile sig_atomic_t g_shutdown_flag;

// Setup signal handler for a specific signal
void setup_signal_handler(int signal, SignalHandler handler);

// Setup common signals (SIGINT, SIGTERM) for graceful shutdown
void setup_graceful_shutdown(void);

// Check if shutdown was requested
int should_shutdown(void);

// Block all signals (for worker threads)
void block_all_signals(void);

// ============================================================================
// 8. TIME/DATE FUNCTIONS
// ============================================================================

typedef struct {
    time_t raw_time;
    struct tm *local_time;
    char formatted[256];
} TimeInfo;

// Get current time
TimeInfo* get_current_time(void);

// Format time with custom format string
char* format_time(time_t t, const char *format);

// Get timestamp string (YYYY-MM-DD HH:MM:SS)
char* get_timestamp(void);

// Parse time string to time_t
time_t parse_time(const char *time_str, const char *format);

// Calculate elapsed time in seconds
double elapsed_seconds(time_t start, time_t end);

// Free TimeInfo
void free_time_info(TimeInfo *ti);

// ============================================================================
// 9. WORK STRUCTURE (like your midterm)
// ============================================================================

typedef struct {
    int param[3];
    int result;
    void *user_data;
} Work;

// Create work array
Work* create_work_array(int count);

// Read work from stdin (returns count)
int read_work_stdin(Work **work, int *capacity);

// ============================================================================
// 10. PTHREAD HELPERS
// ============================================================================

typedef struct {
    pthread_t *tids;
    int count;
    int capacity;
} ThreadArray;

// Create thread array
ThreadArray* thread_array_create(int initial_capacity);

// Add thread to array (auto-resizes)
void thread_array_add(ThreadArray *ta, pthread_t tid);

// Join all threads
void thread_array_join_all(ThreadArray *ta);

// Free thread array
void thread_array_free(ThreadArray *ta);

#endif // EXAM_HELPERS_H