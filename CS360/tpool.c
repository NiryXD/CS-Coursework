#include "tpool.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// Job structure for work queue
typedef struct job {
    int workVal;
    int index;
    Executor executor;
} job_t;

// Thread pool structure
typedef struct thread_pool {
    pthread_t *thread;
    int nThread;
    
    // Work queue
    job_t *jobQ;
    int Qsize;
    int Qcapacity;
    int Qfront;
    int Qback;
    
    // Results array
    uint64_t *results;
    int resultC;
    int resultexpect;
    
    // Synchronization
    pthread_mutex_t mutex;
    pthread_cond_t workCond;
    pthread_cond_t doneCond;
    
    // done flag
    bool done;
} thread_pool_t;

// Worker thread function
// Following the worker pattern from the write-up (lines 76-81)
static void *workerThread(void *arg) {
    thread_pool_t *pool = (thread_pool_t *)arg;
    
    while (1) {
        // Lock mutex before checking queue (lines 77-78)
        pthread_mutex_lock(&pool->mutex);
        
        // Wait for work or done
        while (pool->Qsize == 0 && !pool->done) {
            pthread_cond_wait(&pool->workCond, &pool->mutex);
        }
        
        // Check for done
        if (pool->done) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }
        
        // Pop job from queue (line 79)
        job_t job = pool->jobQ[pool->Qfront];
        pool->Qfront = (pool->Qfront + 1) % pool->Qcapacity;
        pool->Qsize--;
        
        // Unlock mutex after getting job (line 80)
        pthread_mutex_unlock(&pool->mutex);
        
        // Execute the job outside of critical section
        uint64_t result = job.executor(job.workVal);
        
        // Store result
        pthread_mutex_lock(&pool->mutex);
        pool->results[job.index] = result;
        pool->resultC++;
        
        // Signal if all work is done
        if (pool->resultC == pool->resultexpect) {
            pthread_cond_signal(&pool->doneCond);
        }
        
        pthread_mutex_unlock(&pool->mutex);
    }
    
    return NULL;
}

// Thread pool functions
void *thread_pool_open(int nThread) {
    // Validate number of thread
    if (nThread < 1 || nThread > 32) {
        return NULL;
    }
    
    thread_pool_t *pool = (thread_pool_t *)calloc(1, sizeof(thread_pool_t));
    if (!pool) {
        return NULL;
    }
    
    pool->nThread = nThread;
    pool->done = false;
    
    // Allocate thread array
    pool->thread = (pthread_t *)calloc(nThread, sizeof(pthread_t));
    if (!pool->thread) {
        free(pool);
        return NULL;
    }
    
    // Allocate job queue - make it reasonably sized
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
    
    // Initialize synchronization
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
    
    // Create worker thread
    for (int i = 0; i < nThread; i++) {
        if (pthread_create(&pool->thread[i], NULL, workerThread, pool) != 0) {
            // Clean up on failure
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

uint64_t *thread_pool_execute(void *handle, const int *workVals, int nWork, const Executor executor) {
    if (!handle || !workVals || nWork <= 0 || !executor) {
        return NULL;
    }
    
    thread_pool_t *pool = (thread_pool_t *)handle;
    
    // Allocate results array
    uint64_t *results = (uint64_t *)calloc(nWork, sizeof(uint64_t));
    if (!results) {
        return NULL;
    }
    
    pool->results = results;
    pool->resultC = 0;
    pool->resultexpect = nWork;
    
    // Add all jobs to queue following the pattern from the write-up (lines 65-82)
    // This minimizes mutex thrashing by filling the queue while holding the lock
    int i = 0;
    while (i < nWork) {
        pthread_mutex_lock(&pool->mutex);
        
        // Fill queue as much as possible while holding the mutex
        while (pool->Qsize < pool->Qcapacity && i < nWork) {
            job_t job;
            job.workVal = workVals[i];
            job.index = i;
            job.executor = executor;
            
            pool->jobQ[pool->Qback] = job;
            pool->Qback = (pool->Qback + 1) % pool->Qcapacity;
            pool->Qsize++;
            i++;  // Only increment after successfully pushing job
        }
        
        // Signal workers that there's work
        pthread_cond_broadcast(&pool->workCond);
        pthread_mutex_unlock(&pool->mutex);
        
        // If we still have work but queue was full, wait a bit
        if (i < nWork) {
            usleep(1000); // Small delay to let workers process
        }
    }
    
    // Wait for all work to complete
    pthread_mutex_lock(&pool->mutex);
    while (pool->resultC < nWork) {
        pthread_cond_wait(&pool->doneCond, &pool->mutex);
    }
    pthread_mutex_unlock(&pool->mutex);
    
    return results;
}

void thread_pool_close(void *handle) {
    if (!handle) {
        return;
    }
    
    thread_pool_t *pool = (thread_pool_t *)handle;
    
    // Signal done
    pthread_mutex_lock(&pool->mutex);
    pool->done = true;
    pthread_cond_broadcast(&pool->workCond);
    pthread_mutex_unlock(&pool->mutex);
    
    // Join all thread
    for (int i = 0; i < pool->nThread; i++) {
        pthread_join(pool->thread[i], NULL);
    }
    
    // Clean up
    pthread_cond_destroy(&pool->doneCond);
    pthread_cond_destroy(&pool->workCond);
    pthread_mutex_destroy(&pool->mutex);
    
    free(pool->jobQ);
    free(pool->thread);
    free(pool);
}

// Hash functions
// Following the FNV-1a algorithm from the write-up (lines 305-309)
uint64_t hash32(int fd) {
    // u32 hashing <- 2,166,136,261
    uint32_t hashing = 2166136261U;
    // Prime multiplier for 32-bit: 16,777,619
    uint32_t prime = 16777619U;
    
    unsigned char buffer[4096];
    ssize_t bytes_read;
    
    // Reset file position to beginning
    lseek(fd, 0, SEEK_SET);
    
    // for each byte in buffer
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            // hashing <- (hashing ⊕ byte) × 16,777,619
            hashing = (hashing ^ buffer[i]) * prime;
        }
    }
    
    // Return as uint64_t with upper 32 bits cleared to 0
    return (uint64_t)hashing;
}

// Following the FNV-1a algorithm from the write-up (lines 310-314)
uint64_t hash64(int fd) {
    // u64 hashing <- 14,695,981,039,346,656,037
    uint64_t hashing = 14695981039346656037ULL;
    // Prime multiplier for 64-bit: 1,099,511,628,211
    uint64_t prime = 1099511628211ULL;
    
    unsigned char buffer[4096];
    ssize_t bytes_read;
    
    // Reset file position to beginning
    lseek(fd, 0, SEEK_SET);
    
    // for each byte in buffer
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; i++) {
            // hashing <- (hashing ⊕ byte) × 1,099,511,628,211
            hashing = (hashing ^ buffer[i]) * prime;
        }
    }
    
    return hashing;
}