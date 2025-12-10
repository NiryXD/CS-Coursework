#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_RESULT_SIZE 256
#define QUEUE_CAPACITY 100

// Plugin function types
typedef void (*INIT_FUNC)(void);
typedef int (*PROCESS_FUNC)(const char *data, int size, char *result);
typedef void (*CLEANUP_FUNC)(void);

struct WorkItem {
    char *filename;
    int index;  // To maintain order in results
};

struct WorkQueue {
    struct WorkItem items[QUEUE_CAPACITY];
    int front, rear, size;
};

struct ThreadPool {
    pthread_t *threads;
    int num_threads;
    
    struct WorkQueue queue;
    pthread_mutex_t queue_lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
    
    char **results;  // Array of result strings
    int total_files;
    int completed;
    pthread_mutex_t result_lock;
    pthread_cond_t all_done;
    
    PROCESS_FUNC process_func;
    bool shutdown;
};

/* 
 * WORKER THREAD FUNCTION
 * This is the heart of our thread pool - each worker runs this function
 * Key concepts: 
 * - Continuous loop checking for work
 * - Proper synchronization when accessing shared queue
 * - Memory mapping for efficient file processing
 */
void *worker_thread(void *arg) {
    struct ThreadPool *pool = (struct ThreadPool *)arg;
    struct WorkItem work;
    
    while (1) {
        // STEP 1: Get work from the queue (Consumer pattern)
        pthread_mutex_lock(&pool->queue_lock);
        
        // Wait while queue is empty AND we're not shutting down
        // This is the classic condition variable pattern - always use while loop, not if!
        // Why? Spurious wakeups can occur, so we must recheck the condition
        while (pool->queue.size == 0 && !pool->shutdown) {
            pthread_cond_wait(&pool->not_empty, &pool->queue_lock);
        }
        
        // Check if we're shutting down with no more work
        if (pool->shutdown && pool->queue.size == 0) {
            pthread_mutex_unlock(&pool->queue_lock);
            break;
        }
        
        // Dequeue work item (circular queue pattern)
        work = pool->queue.items[pool->queue.front];
        pool->queue.front = (pool->queue.front + 1) % QUEUE_CAPACITY;
        pool->queue.size--;
        
        // Signal producer that queue has space now
        pthread_cond_signal(&pool->not_full);
        pthread_mutex_unlock(&pool->queue_lock);
        
        // STEP 2: Process the file (outside of lock for better concurrency!)
        // Open file for memory mapping
        int fd = open(work.filename, O_RDONLY);
        if (fd < 0) {
            // Store error message in result
            pthread_mutex_lock(&pool->result_lock);
            pool->results[work.index] = malloc(MAX_RESULT_SIZE);
            snprintf(pool->results[work.index], MAX_RESULT_SIZE, 
                    "ERROR: Cannot open %s", work.filename);
            pool->completed++;
            
            // Check if we're the last one to complete
            if (pool->completed == pool->total_files) {
                pthread_cond_signal(&pool->all_done);
            }
            pthread_mutex_unlock(&pool->result_lock);
            
            free(work.filename);  // Don't forget to free the filename!
            continue;
        }
        
        // Get file size for mmap
        struct stat st;
        if (fstat(fd, &st) < 0) {
            close(fd);
            // Similar error handling as above
            pthread_mutex_lock(&pool->result_lock);
            pool->results[work.index] = malloc(MAX_RESULT_SIZE);
            snprintf(pool->results[work.index], MAX_RESULT_SIZE, 
                    "ERROR: Cannot stat %s", work.filename);
            pool->completed++;
            if (pool->completed == pool->total_files) {
                pthread_cond_signal(&pool->all_done);
            }
            pthread_mutex_unlock(&pool->result_lock);
            free(work.filename);
            continue;
        }
        
        // Memory map the file
        // PROT_READ: we only need to read
        // MAP_PRIVATE: changes won't affect the file
        char *data = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
        close(fd);  // Can close fd after mmap
        
        if (data == MAP_FAILED) {
            pthread_mutex_lock(&pool->result_lock);
            pool->results[work.index] = malloc(MAX_RESULT_SIZE);
            snprintf(pool->results[work.index], MAX_RESULT_SIZE, 
                    "ERROR: Cannot map %s", work.filename);
            pool->completed++;
            if (pool->completed == pool->total_files) {
                pthread_cond_signal(&pool->all_done);
            }
            pthread_mutex_unlock(&pool->result_lock);
            free(work.filename);
            continue;
        }
        
        // STEP 3: Call the plugin to process the file
        char *result = malloc(MAX_RESULT_SIZE);
        int ret = pool->process_func(data, st.st_size, result);
        
        // Unmap the file - important to free resources!
        munmap(data, st.st_size);
        
        // STEP 4: Store the result (maintaining order via index)
        pthread_mutex_lock(&pool->result_lock);
        if (ret < 0) {
            snprintf(result, MAX_RESULT_SIZE, 
                    "ERROR: Plugin failed for %s", work.filename);
        }
        pool->results[work.index] = result;  // Store at correct index for ordering
        pool->completed++;
        
        // Signal main thread if all work is done
        if (pool->completed == pool->total_files) {
            pthread_cond_signal(&pool->all_done);
        }
        pthread_mutex_unlock(&pool->result_lock);
        
        free(work.filename);  // Clean up allocated filename
    }
    
    return NULL;
}

/*
 * CREATE THREAD POOL
 * Initialize all components of the thread pool
 * Critical: Initialize mutexes/conditions BEFORE creating threads!
 */
struct ThreadPool *create_pool(int num_threads, int total_files, PROCESS_FUNC func) {
    struct ThreadPool *pool = calloc(1, sizeof(struct ThreadPool));
    if (!pool) return NULL;
    
    // Set basic parameters
    pool->num_threads = num_threads;
    pool->total_files = total_files;
    pool->process_func = func;
    pool->shutdown = false;
    pool->completed = 0;
    
    // Initialize queue (circular buffer)
    pool->queue.front = 0;
    pool->queue.rear = 0;
    pool->queue.size = 0;
    
    // Allocate results array - one slot per file
    pool->results = calloc(total_files, sizeof(char *));
    if (!pool->results) {
        free(pool);
        return NULL;
    }
    
    // Initialize synchronization primitives
    // ALWAYS check return values in production code!
    if (pthread_mutex_init(&pool->queue_lock, NULL) != 0) {
        free(pool->results);
        free(pool);
        return NULL;
    }
    
    if (pthread_mutex_init(&pool->result_lock, NULL) != 0) {
        pthread_mutex_destroy(&pool->queue_lock);
        free(pool->results);
        free(pool);
        return NULL;
    }
    
    // Initialize condition variables
    if (pthread_cond_init(&pool->not_empty, NULL) != 0 ||
        pthread_cond_init(&pool->not_full, NULL) != 0 ||
        pthread_cond_init(&pool->all_done, NULL) != 0) {
        // Cleanup on failure
        pthread_mutex_destroy(&pool->queue_lock);
        pthread_mutex_destroy(&pool->result_lock);
        pthread_cond_destroy(&pool->not_empty);
        pthread_cond_destroy(&pool->not_full);
        pthread_cond_destroy(&pool->all_done);
        free(pool->results);
        free(pool);
        return NULL;
    }
    
    // Allocate thread array
    pool->threads = malloc(num_threads * sizeof(pthread_t));
    if (!pool->threads) {
        pthread_mutex_destroy(&pool->queue_lock);
        pthread_mutex_destroy(&pool->result_lock);
        pthread_cond_destroy(&pool->not_empty);
        pthread_cond_destroy(&pool->not_full);
        pthread_cond_destroy(&pool->all_done);
        free(pool->results);
        free(pool);
        return NULL;
    }
    
    // Create worker threads
    // Important: If thread creation fails, must join already created threads!
    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&pool->threads[i], NULL, worker_thread, pool) != 0) {
            // Set shutdown flag
            pool->shutdown = true;
            
            // Wake up any already created threads so they can exit
            pthread_cond_broadcast(&pool->not_empty);
            
            // Join threads that were successfully created
            for (int j = 0; j < i; j++) {
                pthread_join(pool->threads[j], NULL);
            }
            
            // Full cleanup
            pthread_mutex_destroy(&pool->queue_lock);
            pthread_mutex_destroy(&pool->result_lock);
            pthread_cond_destroy(&pool->not_empty);
            pthread_cond_destroy(&pool->not_full);
            pthread_cond_destroy(&pool->all_done);
            free(pool->threads);
            free(pool->results);
            free(pool);
            return NULL;
        }
    }
    
    return pool;
}

/*
 * DESTROY THREAD POOL
 * Proper shutdown sequence:
 * 1. Set shutdown flag
 * 2. Wake all waiting threads
 * 3. Join all threads
 * 4. Clean up resources
 */
void destroy_pool(struct ThreadPool *pool) {
    if (!pool) return;
    
    // Signal shutdown
    pthread_mutex_lock(&pool->queue_lock);
    pool->shutdown = true;
    pthread_cond_broadcast(&pool->not_empty);  // Wake all workers
    pthread_mutex_unlock(&pool->queue_lock);
    
    // Wait for all threads to finish
    for (int i = 0; i < pool->num_threads; i++) {
        pthread_join(pool->threads[i], NULL);
    }
    
    // Free any remaining items in queue
    while (pool->queue.size > 0) {
        free(pool->queue.items[pool->queue.front].filename);
        pool->queue.front = (pool->queue.front + 1) % QUEUE_CAPACITY;
        pool->queue.size--;
    }
    
    // Destroy synchronization primitives
    pthread_mutex_destroy(&pool->queue_lock);
    pthread_mutex_destroy(&pool->result_lock);
    pthread_cond_destroy(&pool->not_empty);
    pthread_cond_destroy(&pool->not_full);
    pthread_cond_destroy(&pool->all_done);
    
    // Free memory
    free(pool->threads);
    // Note: results are freed by main after writing to file
    free(pool);
}

/*
 * ENQUEUE WORK
 * Producer pattern - adds work to the queue
 */
bool enqueue_work(struct ThreadPool *pool, const char *filename, int index) {
    pthread_mutex_lock(&pool->queue_lock);
    
    // Wait while queue is full (producer-consumer pattern)
    while (pool->queue.size >= QUEUE_CAPACITY) {
        pthread_cond_wait(&pool->not_full, &pool->queue_lock);
    }
    
    // Add item to circular queue
    // Must duplicate filename string since original might go away
    pool->queue.items[pool->queue.rear].filename = strdup(filename);
    pool->queue.items[pool->queue.rear].index = index;
    pool->queue.rear = (pool->queue.rear + 1) % QUEUE_CAPACITY;
    pool->queue.size++;
    
    // Signal that queue is not empty
    pthread_cond_signal(&pool->not_empty);
    pthread_mutex_unlock(&pool->queue_lock);
    
    return true;
}

/*
 * MAIN FUNCTION
 * Orchestrates the entire program:
 * - Argument parsing
 * - Plugin loading
 * - Work distribution
 * - Result collection
 */
int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <num_threads> <plugin.so> <output> <files...>\n", 
                argv[0]);
        return 1;
    }
    
    // Parse arguments
    int num_threads = atoi(argv[1]);
    if (num_threads <= 0 || num_threads > 100) {
        fprintf(stderr, "Invalid number of threads: %s\n", argv[1]);
        return 1;
    }
    
    const char *plugin_name = argv[2];
    const char *output_file = argv[3];
    int num_files = argc - 4;
    
    if (num_files == 0) {
        fprintf(stderr, "No input files specified\n");
        return 1;
    }
    
    // STEP 1: Load the plugin
    // Try different naming conventions (common exam pattern!)
    void *handle = NULL;
    char plugin_path[256];
    
    // Try as-is first
    handle = dlopen(plugin_name, RTLD_LAZY);
    
    // Try with ./ prefix
    if (!handle) {
        snprintf(plugin_path, sizeof(plugin_path), "./%s", plugin_name);
        handle = dlopen(plugin_path, RTLD_LAZY);
    }
    
    // Try with lib prefix and .so suffix
    if (!handle && strncmp(plugin_name, "lib", 3) != 0) {
        snprintf(plugin_path, sizeof(plugin_path), "./lib%s.so", plugin_name);
        handle = dlopen(plugin_path, RTLD_LAZY);
    }
    
    if (!handle) {
        fprintf(stderr, "Failed to load plugin: %s\n", dlerror());
        return 1;
    }
    
    // STEP 2: Get plugin functions
    // Use dlsym to get function pointers from the shared library
    INIT_FUNC init_func = (INIT_FUNC)dlsym(handle, "init");
    PROCESS_FUNC process_func = (PROCESS_FUNC)dlsym(handle, "process_file");
    CLEANUP_FUNC cleanup_func = (CLEANUP_FUNC)dlsym(handle, "cleanup");
    
    if (!process_func) {
        fprintf(stderr, "Plugin missing required process_file function\n");
        dlclose(handle);
        return 1;
    }
    
    // Call init if it exists
    if (init_func) {
        init_func();
    }
    
    // STEP 3: Create thread pool
    struct ThreadPool *pool = create_pool(num_threads, num_files, process_func);
    if (!pool) {
        fprintf(stderr, "Failed to create thread pool\n");
        if (cleanup_func) cleanup_func();
        dlclose(handle);
        return 1;
    }
    
    // STEP 4: Enqueue all files
    // Important: maintain index for ordering results!
    for (int i = 0; i < num_files; i++) {
        if (!enqueue_work(pool, argv[4 + i], i)) {
            fprintf(stderr, "Failed to enqueue work\n");
            destroy_pool(pool);
            if (cleanup_func) cleanup_func();
            dlclose(handle);
            return 1;
        }
    }
    
    // STEP 5: Wait for all work to complete
    pthread_mutex_lock(&pool->result_lock);
    while (pool->completed < num_files) {
        pthread_cond_wait(&pool->all_done, &pool->result_lock);
    }
    pthread_mutex_unlock(&pool->result_lock);
    
    // STEP 6: Write results to output file IN ORDER
    FILE *output = fopen(output_file, "w");
    if (!output) {
        fprintf(stderr, "Failed to open output file: %s\n", output_file);
    } else {
        for (int i = 0; i < num_files; i++) {
            fprintf(output, "File %s: %s\n", argv[4 + i], 
                   pool->results[i] ? pool->results[i] : "No result");
        }
        fclose(output);
        printf("Results written to %s\n", output_file);
    }
    
    // STEP 7: Cleanup everything
    // Free results
    for (int i = 0; i < num_files; i++) {
        free(pool->results[i]);
    }
    free(pool->results);
    
    // Destroy pool
    destroy_pool(pool);
    
    // Cleanup plugin
    if (cleanup_func) {
        cleanup_func();
    }
    dlclose(handle);
    
    return 0;
}