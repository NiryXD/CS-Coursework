#include "tpool.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

// This file implements a simple thread pool. A thread pool lets the
// program hand off small "jobs" to a group of worker threads so the
// work can be done in parallel. The main parts are:
// - a job type (`job_t`) describing a unit of work,
// - a pool struct (`thread_pool_t`) that keeps threads, a job queue,
//   and synchronization primitives (mutex + condition variables),
// - worker threads that loop, take jobs from the queue, run them,
//   and store results, and
// - public functions to open the pool, submit work, and close it.

typedef struct jobStructure {
    int workVal;
    int index;
    Executor executor;
}job_t;

typedef struct threadPool {
    pthread_t *thread;
    int nThread;

    job_t *jobQ;
    int Qsize;
    int Qcapacity;
    int Qfront;
    int Qback;

    uint64_t *result;
    int resultC;
    int resultexpect;

    pthread_mutex_t mutex;
    pthread_cond_t workCond;
    pthread_cond_t doneCond;

    bool done;

} thread_pool_t;

// Layman notes about the fields above:
// - `thread` is an array of worker thread IDs. `nThread` is how many.
// - The job queue is implemented as a circular buffer using `jobQ` and
//   indices `Qfront`/`Qback` plus `Qsize`/`Qcapacity`.
// - `result` is an array where workers place their outputs. `resultC`
//   counts how many results are finished; `resultexpect` is how many
//   we need for the current batch of work.
// - `mutex` protects shared state; `workCond` wakes workers when jobs
//   are available; `doneCond` notifies the submitter when all results
//   are ready. `done` tells workers to exit when true.

static void *workerThread (void *arg) {
    thread_pool_t *pool = (thread_pool_t *)arg;

    for (;;) {
        pthread_mutex_lock(&pool->mutex);

        // Wait while there are no jobs and the pool is not shutting down.
        // `pthread_cond_wait` atomically unlocks the mutex and sleeps.
        // When it returns, the mutex is locked again and we re-check the
        // condition in the while-loop (standard condition-variable pattern).
        while (pool->Qsize == 0 && !pool->done) {
            pthread_cond_wait(&pool->workCond, &pool->mutex);
        }

        if (pool->done && pool->Qsize == 0) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }


    // remove one job from q
        job_t job = pool->jobQ[pool->Qfront];
        pool->Qfront = (pool->Qfront + 1) % pool->Qcapacity;
        pool->Qsize--;

        pthread_mutex_unlock(&pool->mutex);
        uint64_t results = job.executor(job.workVal);

        pthread_mutex_lock(&pool->mutex);
        pool->result[job.index] = results;
        pool->resultC++;

        if (pool->resultC == pool->resultexpect) {
            pthread_cond_signal(&pool->doneCond);
        }
        pthread_mutex_unlock(&pool->mutex);
    }
    return NULL;
}

// Worker thread summary in plain terms:
// - Sleep until there's work or the pool is closing.
// - If closing and no more jobs, exit the loop.
// - Otherwise pop one job from the queue, run its `executor` function
//   (outside the mutex so other threads can run), then store the
//   result and signal when all expected results are in.

void *thread_pool_open(int nThread) {
    // err check
    if (nThread < 1 || nThread > 32) {
        return NULL;
    }

    thread_pool_t *pool = (thread_pool_t *)calloc(1, sizeof(thread_pool_t));
    if (!pool) {
        return NULL;
    }

    pool->nThread = nThread;
    pool->done = false;


    // allocate worker array
    pool->thread = (pthread_t *)calloc(nThread, sizeof(pthread_t));
    if (!pool->thread) {
        free(pool);
        return NULL;
    }


        // Set up a fixed-size circular queue for jobs. 1024 is an
        // arbitrary capacity that should be enough for most uses in this
        // class project. The queue stores `job_t` entries.
        pool->Qcapacity = 1024;
        pool->jobQ = (job_t *)calloc(pool->Qcapacity, sizeof(job_t));
        if (!pool->jobQ) {
            free(pool->thread);
            free(pool);
            return NULL;
    }

    pool->Qsize = 0;
    pool->Qfront = 0;
    pool->Qback = 0;

    if (pthread_mutex_init(&pool->mutex, NULL) != 0) {
        free(pool->jobQ);
        free(pool->thread);
        free(pool);
        return NULL;
    }

    if (pthread_cond_init(&pool->workCond, NULL) != 0) {
        pthread_mutex_destroy(&pool->mutex);
        free(pool->jobQ);
        free(pool->thread);
        free(pool);
        return NULL;
    }

    if (pthread_cond_init(&pool->doneCond, NULL) != 0) {
        pthread_cond_destroy(&pool->workCond);
        pthread_mutex_destroy(&pool->mutex);
        free(pool->jobQ);
        free(pool->thread);
        free(pool);
        return NULL;
    }


    // worker threads
    for (int i = 0; i  < nThread; i++) {
        if (pthread_create(&pool->thread[i], NULL, workerThread, pool) != 0) {
            pool->done = true;
            pthread_cond_broadcast(&pool->workCond);

            for (int j = 0; j < i; j++) {
                pthread_join(pool->thread[j], NULL);
            }

            pthread_cond_destroy(&pool->doneCond);
            pthread_cond_destroy(&pool->workCond);
            pthread_mutex_destroy(&pool->mutex);
            free(pool->jobQ);
            free(pool->thread);
            free(pool);
            return NULL;
        }
    }

    return pool;
}

// `thread_pool_open` summary:
// - Validate input and allocate the pool structure.
// - Initialize the job queue, mutex, and condition variables.
// - Start `nThread` worker threads that will wait for work.
// - If any initialization step fails, clean up and return NULL.

uint64_t *thread_pool_execute(void *handle, const int *workVal, int nWork, const Executor executor) {
    if (!handle || !workVal || nWork <= 0 || !executor) {
        return NULL;
    }

    thread_pool_t *pool = (thread_pool_t *)handle;

    uint64_t *results = (uint64_t *)calloc(nWork, sizeof(uint64_t));
    if (!results) {
        return NULL;
    }

    pool->result = results;
    pool->resultC = 0;
    pool->resultexpect = nWork;

    int i = 0;
    while (i < nWork) {
        pthread_mutex_lock(&pool->mutex);


    // feed jobs into q
        while (pool->Qsize < pool->Qcapacity && i < nWork) {
            job_t job;
            job.workVal = workVal[i];
            job.index = i;
            job.executor = executor;

            pool->jobQ[pool->Qback] = job;
            pool->Qback = (pool->Qback + 1)  % pool->Qcapacity;
            pool->Qsize++;
            i++;
        }
        
        pthread_cond_broadcast(&pool->workCond);
        pthread_mutex_unlock(&pool->mutex);

        if (i < nWork) {
            usleep(1000);
        }
    }

    // Wait until all submitted jobs have produced results. The workers
    // increment `resultC` and signal `doneCond` when all are done.
    pthread_mutex_lock(&pool->mutex);
    while (pool->resultC < nWork) {
        pthread_cond_wait(&pool->doneCond, &pool->mutex);
    }
    pthread_mutex_unlock(&pool->mutex);

    return results;
    }

// `thread_pool_execute` summary:
// - Allocate results array and set the expected count.
// - Push jobs into the queue until all work is queued (waiting briefly
//   when the queue is full).
// - Signal workers that work is available and wait for all results.

void thread_pool_close(void *handle) {
    if (!handle) {
        return;
    }

    thread_pool_t *pool = (thread_pool_t *)handle;

    pthread_mutex_lock(&pool->mutex);
    pool->done = true;
    pthread_cond_broadcast(&pool->workCond);
    pthread_mutex_unlock(&pool->mutex);

    for (int i = 0; i < pool->nThread; i++) {
        pthread_join(pool->thread[i], NULL);
    }

    pthread_cond_destroy(&pool->doneCond);
    pthread_cond_destroy(&pool->workCond);
    pthread_mutex_destroy(&pool->mutex);

    free(pool->jobQ);
    free(pool->thread);
    free(pool);
}

// `thread_pool_close` summary:
// - Mark the pool as done, wake all workers, and join them so we know
//   they finished. Then destroy synchronization objects and free memory.

uint64_t hash32(int fd) {
    // I utlilize ChatGPT to explain the hashing explained in the write up
    uint32_t hashing = 2166136261U;
    uint32_t prime = 16777619U;

    unsigned char buffer[4096];
    ssize_t bytes_read;

    lseek(fd, 0, SEEK_SET);

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            hashing = (hashing ^ buffer[i]) * prime;
        }
    }

    return (uint64_t)hashing;
}

// `hash32` implements the 32-bit FNV-1a hash over the file's bytes.
// It seeks to the start of the file and mixes each byte into the
// hash. The result is returned as a 64-bit value for convenience.

uint64_t hash64(int fd) {
    uint64_t hashing = 14695981039346656037ULL;
    uint64_t prime = 1099511628211ULL;

    unsigned char buffer[4096];
    ssize_t bytes_read;

    lseek(fd, 0, SEEK_SET);

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            hashing = (hashing ^ buffer[i]) * prime;
        }
    }

    return hashing;
}

// `hash64` is the 64-bit FNV-1a variant and works like `hash32` but
// with a larger state. Both hash functions are quick ways to get a
// reasonably distributed fingerprint of file contents.



// Write your five (5) functions here
// All are prototyped in tpool.h
