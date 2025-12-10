#include <stdio.h>
#include "hfactor.h"
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <mqueue.h>
#include <semaphore.h>
#include <stdint.h>
#include <string.h>


// Helper Functions

ssize_t writeAll (int fd, const void *buffer, size_t totalBytes ) {
    const char *current = (const char *)buffer;
    size_t remain = totalBytes;
    while (remain > 0 ) {
        ssize_t wrote = write(fd, current, remain);
        if (wrote <= 0)
        return -1;

        current += wrote;
        remain -= (size_t)wrote;
    }
    return (ssize_t)totalBytes;
}

ssize_t readAll (int fd, void *buffer, size_t totalBytes) {
    char *current = (char *)buffer;
    size_t remain = totalBytes;
    
    while (remain > 0) {
        ssize_t r = read(fd, current, remain);
        if (r <= 0) { 
        return -1;
        }
        current += r;
        remain -= (size_t)r;
    }
    return (ssize_t)totalBytes;
}

int hfactor_process(uint64_t target)
{
    mqd_t queue = mq_open(QUEUE_NAME, O_RDONLY);
    if ( queue == (mqd_t)-1) {
        return -1;
    }

    sem_t *guard = sem_open(SEMAPHORE_NAME, 0);
    if (guard == SEM_FAILED) {
        mq_close(queue);
        return -1;
    }

    int sharedMem = shm_open(SHM_NAME, O_RDWR, 0);
    if (sharedMem < 0) {
        sem_close(guard);
        mq_close(queue);
        return -1;
    }

    int resultCode = 0;

    for (;;) {
        uint64_t testFactor = 0;

        ssize_t bytesGot = mq_receive(queue, (char *)&testFactor, sizeof(testFactor), NULL);
        if (bytesGot < 0) {
            resultCode = -1;
            break;
        }
        if (testFactor == 0){
            break;
        }

        if (target != 0 && testFactor >= 2 && testFactor <= target / 2 && (target % testFactor) == 0){
            if (sem_wait(guard) < 0) {
                resultCode = -1;
                break;
            }

        uint64_t factorCount = 0;
        if (lseek(sharedMem, 0, SEEK_SET) < 0 || readAll(sharedMem, &factorCount, sizeof(factorCount)) != (ssize_t)sizeof(factorCount)) {
          resultCode = -1;  
          sem_post(guard);
          break;
        }

        off_t nTotalBytes = (off_t)(8 + 8 * (factorCount + 1));
        if (ftruncate(sharedMem, nTotalBytes) < 0) {
            resultCode = -1;
            sem_post(guard);
            break;
        }

        off_t offset = (off_t)(8 + 8 * factorCount);
        if (lseek(sharedMem, offset, SEEK_SET) < 0 || writeAll(sharedMem, &testFactor, sizeof(testFactor)) != (ssize_t)sizeof(testFactor)) {
            resultCode = -1;
            sem_post(guard);
            break;
        }

        factorCount += 1;
        if (lseek(sharedMem, 0, SEEK_SET) < 0 || writeAll (sharedMem, &factorCount, sizeof(factorCount)) != (ssize_t)sizeof(factorCount)) {
            resultCode = -1;
            sem_post(guard);
            break;
        }

        if (sem_post(guard) < 0) {
            resultCode = -1;
            break;
        }
        }
    }

    mq_close(queue);
    sem_close(guard);
    close(sharedMem);

    return resultCode;
}

int hfactor_init(mqd_t *q, sem_t **sem, int *shm)
{
    // https://man7.org/linux/man-pages/man2/open.2.html
    // I had ChatGPT explain permissions to me, aswell as functions that would help me implement int
    // Example being how memset if used to initalize the structures memory. 

    mq_unlink(QUEUE_NAME);
    sem_unlink(SEMAPHORE_NAME);
    shm_unlink(SHM_NAME);

    struct mq_attr attribute;
    memset(&attribute, 0, sizeof(attribute));
    attribute.mq_maxmsg = 64;
    attribute.mq_msgsize = sizeof(uint64_t);

    mqd_t queue = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0666, &attribute);
    if (queue == (mqd_t)-1) {
        return -1;
    }

    sem_t *guard = sem_open(SEMAPHORE_NAME, O_CREAT, 0666, 1);
    if (guard == SEM_FAILED) {
        mq_close(queue);
        mq_unlink(QUEUE_NAME);
        return -1;
    }

    int sharedMem = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (sharedMem < 0) {
        sem_close(guard);
        sem_unlink(SEMAPHORE_NAME);
        mq_close(queue);
        mq_unlink(QUEUE_NAME);
        return -1;
    }

    if (ftruncate(sharedMem, 8) < 0) {
        close(sharedMem);
        sem_close(guard);
        sem_unlink(SEMAPHORE_NAME);
        mq_close(queue);
        mq_unlink(QUEUE_NAME);
        shm_unlink(SHM_NAME);
        return -1;
    }

    uint64_t zero = 0;
    if (lseek(sharedMem, 0, SEEK_SET) < 0 || writeAll(sharedMem, &zero, sizeof(zero)) != (ssize_t)sizeof(zero)) {
        close(sharedMem);
        sem_close(guard);
        sem_unlink(SEMAPHORE_NAME);
        mq_close(queue);
        mq_unlink(QUEUE_NAME);
        shm_unlink(SHM_NAME);
        return -1;
    }

    *q = queue;
    *sem = guard;
    *shm = sharedMem;
    return 0;

}

void hfactor_deinit(mqd_t q, sem_t *sem, int shm)
{
    mqd_t queue = q;
    sem_t *guard = sem;
    int sharedMem = shm;

    if (queue != (mqd_t)-1) {
        mq_close(queue);
    }
    if (guard && guard != SEM_FAILED) {
        sem_close(guard);
    }
    if (sharedMem >= 0) {
        close(sharedMem);
    }

    mq_unlink(QUEUE_NAME);
    sem_unlink(SEMAPHORE_NAME);
    shm_unlink(SHM_NAME);
}

int hfactor_report(const char *reporter, int shm)
{
    uint64_t factorCount = 0;
    if (lseek(shm, 0, SEEK_SET) < 0 || readAll(shm, &factorCount, sizeof(factorCount)) != (ssize_t)sizeof(factorCount)) {
        return -1;
    }

    int reportPipe[2];
    if(pipe(reportPipe) < 0) {
        return -1;
    }

    pid_t childPid = fork();
    if (childPid < 0) {
        close(reportPipe[0]);
        close(reportPipe[1]);
        return -1;
    }

    if(childPid == 0) {
        close(reportPipe[1]);

        if(dup2(reportPipe[0], STDIN_FILENO) < 0) {
            _exit(67);
        }
        close(reportPipe[0]);

        execl(reporter, reporter, (char *)NULL);
        _exit(420);
    }

    close(reportPipe[0]);

    if (writeAll(reportPipe[1], &factorCount, sizeof(factorCount)) != (ssize_t)sizeof(factorCount)) {
        close(reportPipe[1]);
        (void)waitpid(childPid, NULL, 0);
        return -1;
    }

    for (uint64_t i = 0; i < factorCount; i++) {
        uint64_t factorValue = 0;
        off_t offset = (off_t)(8 + 8 * i);

        if (lseek(shm, offset, SEEK_SET) < 0 || 
        readAll(shm, &factorValue, sizeof(factorValue)) != (ssize_t)sizeof(factorValue) || 
        writeAll(reportPipe[1], &factorValue, sizeof(factorValue)) != (ssize_t)sizeof(factorValue)) {
            close(reportPipe[1]);
            (void)waitpid(childPid, NULL, 0);
            return -1;
        }
    }
    close(reportPipe[1]);
    (void)waitpid(childPid, NULL, 0);
    return 0;
}

int hfactor_calculate(const char *reporter, uint64_t target, uint64_t num_workers)
{
    mqd_t queue = (mqd_t)-1;
    sem_t *guard = SEM_FAILED;
    int sharedMem = -1;

    if (hfactor_init(&queue, &guard, &sharedMem) < 0) {
        return -1;
    }

    if (guard != NULL && guard != SEM_FAILED){
        sem_close(guard);
    }
    guard = NULL;

    if (sharedMem >= 0) {
        close(sharedMem);
    }
    sharedMem = -1;



    uint64_t created = 0;
    uint64_t i = 0;
    for (i = 0; i < num_workers; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            break;
        }

        if (pid == 0) {
            int childResult = hfactor_process(target);
            if (childResult == 0) {
                _exit(0);
            }
            else {
                _exit(69);
            }
        }
        created += 1;
    }

    uint64_t maxFactor;
    if (target >= 4) {
        maxFactor = target / 2;
    }
    else {
        maxFactor = 1;
    }

    uint64_t j;
    for (j = 2; j <= maxFactor; j++) {
        int alright = mq_send(queue, (const char *)&j, sizeof(j), 0);
        if (alright < 0) {
            break;
        }
    }

    uint64_t k = 0;
    for (i = 0; i < created; i++) {
        (void)mq_send(queue, (const char *)&k, sizeof(k), 0);
    }

    for (i = 0; i < created; i++) {
        (void)waitpid(-1, NULL, 0);
    }

    int shmReport = shm_open(SHM_NAME, O_RDWR, 0);
    if (shmReport < 0) {
        hfactor_deinit(queue, NULL, -1);
        return -1;
    }

    int reportResult = hfactor_report(reporter, shmReport);
    hfactor_deinit(queue, NULL, shmReport);
    if (reportResult < 0) {
        return -1;
    }
    return 0;
}
