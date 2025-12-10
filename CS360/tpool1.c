#include "tpool.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

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

static void *workerThread (void *arg) {
    thread_pool_t *pool = (thread_pool_t *)arg;

    for (;;) {
        pthread_mutex_lock(&pool->mutex);

        while (pool->Qsize == 0 && !pool->done) {
            pthread_cond_wait(&pool->workCond, &pool->mutex);
        }

        if (pool->done && pool->Qsize == 0) {
            pthread_mutex_unlock(&pool->mutex);
            break;
        }

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

void *thread_pool_open(int nThread) {
    if (nThread < 1 || nThread > 32) {
        return NULL;
    }

    thread_pool_t *pool = (thread_pool_t *)calloc(1, sizeof(thread_pool_t));
    if (!pool) {
        return NULL;
    }

    pool->nThread = nThread;
    pool->done = false;

    pool->thread = (pthread_t *)calloc(nThread, sizeof(pthread_t));
    if (!pool->thread) {
        free(pool);
        return NULL;
    }


        pool->Qcapacity = 1024;
        pool->jobQ = (job_t *)calloc(pool->Qcapacity, sizeof(job_t));
        if (!pool->thread){
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

uint64_t hash32(int fd) {
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



// Write your five (5) functions here
// All are prototyped in tpool.h
