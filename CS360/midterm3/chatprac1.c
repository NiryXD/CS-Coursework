#include "tpool.h"

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

// Each job in the queue: a work value, index of result, and executor function
typedef struct jobStructure {
    int workVal;        // integer work item (in our use case: a file descriptor)
    int index;          // index into the result array
    Executor executor;  // function that computes the result for this work item
} job_t;

// Thread pool internal representation
typedef struct threadPool {
    pthread_t *threads;   // array of worker threads
    int nThread;          // number of worker threads

    // Circular job queue
    job_t *jobQ;          // array of jobs
    int Qcapacity;        // maximum number of jobs
    int Qsize;            // current number of jobs
    int Qfront;           // index of next job to pop
    int Qback;            // index where next job will be pushed

    // Results for current batch (owned by the caller, but filled by workers)
    uint64_t *result;
    int resultC;          // how many results we have written
    int resultexpect;     // how many results we expect (nWork)

    // Synchronization
    pthread_mutex_t mutex;    // protects queue and result counters
    pthread_cond_t workCond;  // signaled when there is jobQ work
    pthread_cond_t doneCond;  // signaled when all results are ready

    bool done;            // set to true to tell workers to exit
} thread_pool_t;

// Helper to check if queue is empty
static bool queue_empty(const thread_pool_t *pool)
{
    return pool->Qsize == 0;
}

// Helper to check if queue is full
static bool queue_full(const thread_pool_t *pool)
{
    return pool->Qsize >= pool->Qcapacity;
}

// Push a job into the circular queue. Returns false if full.
static bool queue_push(thread_pool_t *pool, job_t job)
{
    if (queue_full(pool)) {
        return false;
    }
    pool->jobQ[pool->Qback] = job;
    pool->Qback = (pool->Qback + 1) % pool->Qcapacity;
    pool->Qsize++;
    return true;
}

// Pop a job from the circular queue. Returns false if empty.
static bool queue_pop(thread_pool_t *pool, job_t *out)
{
    if (queue_empty(pool)) {
        return false;
    }
    *out = pool->jobQ[pool->Qfront];
    pool->Qfront = (pool->Qfront + 1) % pool->Qcapacity;
    pool->Qsize--;
    return true;
}

// Worker thread function:
// - waits for work
// - pops a job
// - runs executor(workVal)
// - stores the result in the shared result array
static void *workerThread(void *arg)
{
    thread_pool_t *pool = (thread_pool_t *)arg;

    for (;;) {
        pthread_mutex_lock(&pool->mutex);

        // Wait until there is work OR the pool is marked as done.
        // We use a while loop because pthread_cond_wait can wake up spuriously.
        while (!pool->done && queue_empty(pool)) {
            pthread_cond_wait(&pool->workCond, &pool->mutex);
        }

        // If the pool is flagged done and there is no queued work, exit.
        if (pool->done && queue_empty(pool)) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }

        // Pop one job from the queue.
        job_t job;
        if (!queue_pop(pool, &job)) {
            // Should not happen: we checked not empty above.
            pthread_mutex_unlock(&pool->mutex);
            continue;
        }

        pthread_mutex_unlock(&pool->mutex);

        // Do the work outside the lock so other threads can run concurrently.
        uint64_t r = job.executor(job.workVal);

        // Store the result under the lock.
        pthread_mutex_lock(&pool->mutex);
        if (pool->result &&
            job.index >= 0 &&
            job.index < pool->resultexpect) {
            pool->result[job.index] = r;
        }
        pool->resultC++;

        // If we produced all expected results, wake up any waiting thread.
        if (pool->resultC == pool->resultexpect) {
            pthread_cond_signal(&pool->doneCond);
        }
        pthread_mutex_unlock(&pool->mutex);
    }

    return NULL;
}

// Create and initialize the thread pool
void *thread_pool_open(int nthreads)
{
    // Basic sanity check on number of threads
    if (nthreads < 1 || nthreads > 32) {
        return NULL;
    }

    thread_pool_t *pool = (thread_pool_t *)calloc(1, sizeof(thread_pool_t));
    if (!pool) {
        return NULL;
    }

    pool->nThread = nthreads;
    pool->done = false;

    // Allocate thread array
    pool->threads = (pthread_t *)calloc(nthreads, sizeof(pthread_t));
    if (!pool->threads) {
        free(pool);
        return NULL;
    }

    // Allocate job queue with a fixed capacity
    pool->Qcapacity = 1024;
    pool->jobQ = (job_t *)calloc(pool->Qcapacity, sizeof(job_t));
    if (!pool->jobQ) {
        free(pool->threads);
        free(pool);
        return NULL;
    }
    pool->Qsize = 0;
    pool->Qfront = 0;
    pool->Qback = 0;

    // Initialize synchronization primitives
    if (pthread_mutex_init(&pool->mutex, NULL) != 0) {
        free(pool->jobQ);
        free(pool->threads);
        free(pool);
        return NULL;
    }

    if (pthread_cond_init(&pool->workCond, NULL) != 0) {
        pthread_mutex_destroy(&pool->mutex);
        free(pool->jobQ);
        free(pool->threads);
        free(pool);
        return NULL;
    }

    if (pthread_cond_init(&pool->doneCond, NULL) != 0) {
        pthread_cond_destroy(&pool->workCond);
        pthread_mutex_destroy(&pool->mutex);
        free(pool->jobQ);
        free(pool->threads);
        free(pool);
        return NULL;
    }

    // Create worker threads that will wait on workCond
    for (int i = 0; i < nthreads; i++) {
        if (pthread_create(&pool->threads[i], NULL, workerThread, pool) != 0) {
            // If thread creation fails, shut down everything created so far.
            pool->done = true;
            pthread_cond_broadcast(&pool->workCond);

            for (int j = 0; j < i; j++) {
                pthread_join(pool->threads[j], NULL);
            }
            pthread_cond_destroy(&pool->doneCond);
            pthread_cond_destroy(&pool->workCond);
            pthread_mutex_destroy(&pool->mutex);
            free(pool->jobQ);
            free(pool->threads);
            free(pool);
            return NULL;
        }
    }

    return pool;
}

// Submit a batch of work items to the pool and wait for all results
uint64_t *thread_pool_execute(void *handle,
                              const int *workVal,
                              int nWork,
                              const Executor executor)
{
    if (!handle || !workVal || nWork <= 0 || !executor) {
        return NULL;
    }

    thread_pool_t *pool = (thread_pool_t *)handle;

    // Allocate array to hold results in order
    uint64_t *results = (uint64_t *)calloc(nWork, sizeof(uint64_t));
    if (!results) {
        return NULL;
    }

    // Tell the pool where to store results and how many to expect
    pthread_mutex_lock(&pool->mutex);
    pool->result = results;
    pool->resultC = 0;
    pool->resultexpect = nWork;
    pthread_mutex_unlock(&pool->mutex);

    int i = 0;
    while (i < nWork) {
        pthread_mutex_lock(&pool->mutex);

        // Push as many jobs as we can while there is room in the queue
        while (!queue_full(pool) && i < nWork) {
            job_t job;
            job.workVal = workVal[i];
            job.index = i;
            job.executor = executor;

            queue_push(pool, job);
            i++;
        }

        // Wake worker threads so they can process the jobs
        pthread_cond_broadcast(&pool->workCond);
        pthread_mutex_unlock(&pool->mutex);

        // Optional small sleep if there is more work to feed
        if (i < nWork) {
            usleep(1000); // sleep 1 ms to avoid busy spinning
        }
    }

    // Wait until all results are ready
    pthread_mutex_lock(&pool->mutex);
    while (pool->resultC < pool->resultexpect) {
        pthread_cond_wait(&pool->doneCond, &pool->mutex);
    }
    pthread_mutex_unlock(&pool->mutex);

    // The caller owns the results array and must free it
    return results;
}

// Shut down the thread pool and free all resources
void thread_pool_close(void *handle)
{
    if (!handle) {
        return;
    }

    thread_pool_t *pool = (thread_pool_t *)handle;

    // Tell workers to exit as soon as the queue is empty
    pthread_mutex_lock(&pool->mutex);
    pool->done = true;
    pthread_cond_broadcast(&pool->workCond);
    pthread_mutex_unlock(&pool->mutex);

    // Wait for all worker threads to finish
    for (int i = 0; i < pool->nThread; i++) {
        pthread_join(pool->threads[i], NULL);
    }

    pthread_cond_destroy(&pool->doneCond);
    pthread_cond_destroy(&pool->workCond);
    pthread_mutex_destroy(&pool->mutex);

    free(pool->jobQ);
    free(pool->threads);
    free(pool);
}

// Simple FNV-like 32 bit hash of file contents
uint64_t hash32(int fd)
{
    // 32 bit FNV offset and prime
    uint32_t hashing = 2166136261U;
    uint32_t prime = 16777619U;

    unsigned char buffer[4096];
    ssize_t bytes_read;

    // Always start hashing from the beginning of the file
    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
        return 0;
    }

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            hashing = (hashing ^ buffer[i]) * prime;
        }
    }

    // Return the 32 bit hash stored inside a 64 bit container
    return (uint64_t)hashing;
}

// Simple FNV-like 64 bit hash of file contents
uint64_t hash64(int fd)
{
    // 64 bit FNV offset and prime
    uint64_t hashing = 14695981039346656037ULL;
    uint64_t prime = 1099511628211ULL;

    unsigned char buffer[4096];
    ssize_t bytes_read;

    if (lseek(fd, 0, SEEK_SET) == (off_t)-1) {
        return 0;
    }

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            hashing = (hashing ^ buffer[i]) * prime;
        }
    }

    return hashing;
}
