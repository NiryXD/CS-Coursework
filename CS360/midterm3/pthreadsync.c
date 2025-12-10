#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <pthread.h>

struct List {
    int x;
    int y;
    int z;
    struct List *next;
};

struct Data {
    int result;
    struct List *work_list;
    pthread_mutex_t lock;
};

void *worker(void *arg)
{
    struct Data *data = (struct Data *)arg;
    struct List *t;
    
    /*
     * - This program expects multiple threads to run the `worker` function
     *   at the same time. Each thread takes one job from a shared linked
     *   list (`work_list`), computes a small value from the job (x*y + z),
     *   adds that to a single shared `result`, and then frees the job node.
     *
     * - Because the list and the `result` variable are shared by all
     *   threads, we must protect access to them so two threads don't try to
     *   take the same job or update the result at the same time. We do that
     *   with a mutex (`data->lock`). A mutex is like a bathroom key: only the
     *   thread that holds the key (lock) can access the shared resources.
     *
     * Typical sequence inside each worker thread (very simple):
     *   1. Lock the mutex (take the key).
     *   2. Remove the head job from the shared list and read its fields.
     *   3. Update the shared `result` using that job's data.
     *   4. Unlock the mutex (give the key back).
     *   5. Free the memory for the job node.
     *
     * - Without the mutex two bad things could happen:
     *   * Two threads remove the same node (double-free / wrong result).
     *   * The linked list or the `result` could get corrupted by
     *     simultaneous writes.
     *
     * Note: This code assumes there is at least one job in the list when the
     * worker runs. A production program should check for NULL and handle an
     * empty list (or use a condition variable to wait for work).
     */

    /* take the lock so this thread exclusively accesses the shared data */
    pthread_mutex_lock(&data->lock);

    /*
     * Now that we hold the lock, safely remove the first node from the
     * shared list and update the shared result. Because the lock is held,
     * no other thread can modify `work_list` or `result` right now.
     */
    t = data->work_list;
    data->work_list = t->next;
    data->result += t->x * t->y + t->z;

    /* release the lock so other threads can access the shared data */
    pthread_mutex_unlock(&data->lock);

    /*
     * After unlocking we free the node. Freeing after unlocking is safe here
     * because no other thread will try to access `t` — it was removed from
     * the shared list while we held the lock.
     */
    free(t);

    /*
     * In a typical program, threads are created with `pthread_create` like:
     *   pthread_create(&tid, NULL, worker, &data);
     * and later joined with `pthread_join` so the main thread waits for them.
     */

    return NULL;
}

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dlfcn.h>
#include "work.h"

// Function pointer types for DSO functions
typedef void* (*WORKER_FUNC)(void *);
typedef void (*REPORT_FUNC)(WORK *);

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <worker library>\n", argv[0]);
        return 1;
    }

    // 1. Build path to DSO (try multiple variations)
    void *handle = NULL;
    char path[256];
    
    // Try original path first
    handle = dlopen(argv[1], RTLD_LAZY);
    
    // Try with ./ prefix
    if (!handle) {
        snprintf(path, sizeof(path), "./%s", argv[1]);
        handle = dlopen(path, RTLD_LAZY);
    }
    
    // Try with lib prefix and .so extension
    if (!handle) {
        snprintf(path, sizeof(path), "./lib%s.so", argv[1]);
        handle = dlopen(path, RTLD_LAZY);
    }
    
    if (!handle) {
        fprintf(stderr, "Failed to load DSO: %s\n", dlerror());
        return 1;
    }

    // 2. Get worker and report functions from DSO
    WORKER_FUNC worker = (WORKER_FUNC)dlsym(handle, "worker");
    if (!worker) {
        fprintf(stderr, "Failed to load worker: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }
    
    REPORT_FUNC report = (REPORT_FUNC)dlsym(handle, "report");
    if (!report) {
        fprintf(stderr, "Failed to load report: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }

    // 3. Read 3 integers from stdin
    int param[3];
    if (scanf("%d %d %d", &param[0], &param[1], &param[2]) != 3) {
        fprintf(stderr, "Failed to read 3 integers\n");
        dlclose(handle);
        return 1;
    }

    // 4. Create threads for each parameter
    int num_threads = 3;  // One thread per parameter
    pthread_t *threads = malloc(num_threads * sizeof(pthread_t));
    WORK *works = malloc(num_threads * sizeof(WORK));
    
    if (!threads || !works) {
        fprintf(stderr, "Memory allocation failed\n");
        free(threads);
        free(works);
        dlclose(handle);
        return 1;
    }
    
    // Initialize WORK structures and create threads
    for (int i = 0; i < num_threads; i++) {
        // Each thread gets all params but might process differently
        works[i].param[0] = param[0];
        works[i].param[1] = param[1];
        works[i].param[2] = param[2];
        works[i].result = 0;
        
        if (pthread_create(&threads[i], NULL, worker, &works[i]) != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i);
            // Clean up threads created so far
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            free(threads);
            free(works);
            dlclose(handle);
            return 1;
        }
    }

    // 5. Join all threads
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // 6. Report results using the DSO's report function
    for (int i = 0; i < num_threads; i++) {
        report(&works[i]);
    }

    // 7. Clean up memory
    free(threads);
    free(works);
    dlclose(handle);

    printf("Anything you print in stdout will not be graded.\n");
    printf("You can use it for debugging purposes instead.\n");
    return 0;
}

