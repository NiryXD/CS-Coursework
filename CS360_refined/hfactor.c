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
#include <stdlib.h>


int hfactor_process(uint64_t target)
{
    // Prep the q and sem and sharedMem
    mqd_t queue = mq_open(QUEUE_NAME, O_RDONLY);
    if ( queue == (mqd_t)-1) {
        return -1;
    
    }

    sem_t *guard = sem_open(SEMAPHORE_NAME, 0);
    if (guard == SEM_FAILED) {
        mq_close(queue);
        return -1;
    }

    int sharedMem = shm_open(SHM_NAME, O_RDWR, 0666);
    if (sharedMem == -1) {
        sem_close(guard);
        mq_close(queue);
        return -1;
    }

    for (;;) {
        // main loop
        uint64_t testFactor = 0;

        ssize_t bytes = mq_receive(queue, (char *)&testFactor, sizeof(testFactor), NULL);
        if (bytes == -1 || testFactor == 0)
        break;

        if ( target % testFactor == 0) {
            if (sem_wait(guard) == -1)
            break;
        

        uint64_t factorCount = 0;
        if (lseek(sharedMem, 0, SEEK_SET) < 0 || 
        read(sharedMem, &factorCount, sizeof(factorCount)) != 
        (ssize_t)sizeof(factorCount)) { 
          sem_post(guard);
          break;
        }

        off_t nSize = sizeof(uint64_t) * (factorCount + 2);
        if (ftruncate(sharedMem, nSize) == -1) {
            sem_post(guard);
            break;
        }

        off_t off = (off_t)(sizeof(uint64_t) + factorCount * sizeof(uint64_t));
        if (lseek(sharedMem, off, SEEK_SET) < 0 ||
        write(sharedMem, &testFactor, sizeof(testFactor)) != (ssize_t)sizeof(testFactor)) {
            sem_post(guard);
            break;
        }

        factorCount++;
        if (lseek(sharedMem, 0, SEEK_SET) < 0 || 
        write(sharedMem, &factorCount, sizeof(factorCount)) 
        != (ssize_t)sizeof(factorCount)) {
            sem_post(guard);
            break;
        }

        sem_post(guard);
        
    }
    }

    mq_close(queue);
    sem_close(guard);
    close(sharedMem);
    //cleamn up

    return 0;
}

int hfactor_init(mqd_t *queue, sem_t **guard, int *sharedMem)
{
    // https://man7.org/linux/man-pages/man2/open.2.html
    // I had ChatGPT explain permissions to me, aswell as functions that would help me implement int
    // Example being how memset if used to initalize the structures memory. 

    if (!queue || !guard || !sharedMem)
    return -1;

    // remove
    mq_unlink(QUEUE_NAME);
    sem_unlink(SEMAPHORE_NAME);
    shm_unlink(SHM_NAME);


    struct mq_attr attribute;
    memset(&attribute, 0, sizeof(attribute));
    attribute.mq_flags = 0;
    attribute.mq_maxmsg = 10;
    attribute.mq_msgsize = sizeof(uint64_t);
    attribute.mq_curmsgs = 0;


    // creation of q, sem, sharedMem
    *queue = mq_open(QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR, 0666, &attribute);
    if (*queue == (mqd_t)-1) {
        return -1;
    }

    *guard = sem_open(SEMAPHORE_NAME, O_CREAT | O_EXCL, 0666, 1);
    if (*guard == SEM_FAILED) {
        mq_close(*queue);
        mq_unlink(QUEUE_NAME);
        return -1;
    }

    *sharedMem = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (*sharedMem == -1) {
        sem_close(*guard);
        sem_unlink(SEMAPHORE_NAME);
        mq_close(*queue);
        mq_unlink(QUEUE_NAME);
        return -1;
    }

    uint64_t zero = 0;
    if (ftruncate(*sharedMem, sizeof(zero)) == -1) {
        close(*sharedMem);
        sem_close(*guard);
        sem_unlink(SEMAPHORE_NAME);
        mq_close(*queue);
        mq_unlink(QUEUE_NAME);
        shm_unlink(SHM_NAME);
        return -1;
    }

    if (lseek(*sharedMem, 0, SEEK_SET) < 0 
    || write(*sharedMem, &zero, sizeof(zero)) 
    != (ssize_t)sizeof(zero)) {
        close(*sharedMem);
        sem_close(*guard);
        sem_unlink(SEMAPHORE_NAME);
        mq_close(*queue);
        mq_unlink(QUEUE_NAME);
        shm_unlink(SHM_NAME);
        return -1;
    }
    
    return 0;

}

void hfactor_deinit(mqd_t q, sem_t *sem, int shm)
{

    if (q != (mqd_t)-1)
    mq_close(q);
    if (sem && sem != SEM_FAILED)
    sem_close(sem);
    if (shm >= 0)
    close(shm);

    mq_unlink(QUEUE_NAME);
    sem_unlink(SEMAPHORE_NAME);
    shm_unlink(SHM_NAME);
}

int hfactor_report(const char *reporter, int shm)
{

    int reportPipe[2];
    if(pipe(reportPipe) == -1) {
        return -1;
    }

    pid_t childPid = fork();
    if (childPid == -1) {
        close(reportPipe[0]);
        close(reportPipe[1]);
        return -1;
    }

    if(childPid == 0) { // Child
        close(reportPipe[1]);

        if(dup2(reportPipe[0], STDIN_FILENO) == -1) {
            exit(1);
        }
        close(reportPipe[0]);

        execl(reporter, reporter, (char *)NULL);
        exit(1);
    }

    close(reportPipe[0]);


    uint64_t size = 0;
    if (lseek(shm, 0, SEEK_SET) < 0 ||
    read(shm, &size, sizeof(size)) != (ssize_t)sizeof(size)) {
        close(reportPipe[1]);
        return -1;
    }

    if (write(reportPipe[1], &size, sizeof(size)) != (ssize_t)sizeof(size)) {
        close(reportPipe[1]);
        return -1;
    }

    for (uint64_t i = 0; i < size; i++) {
        uint64_t factorValue = 0;
        off_t offset = (off_t)(sizeof(uint64_t) + i * sizeof(uint64_t));

        if (lseek(shm, offset, SEEK_SET) < 0 || 
        read(shm, &factorValue, sizeof(factorValue)) != (ssize_t)sizeof(factorValue) || 
        write(reportPipe[1], &factorValue, sizeof(factorValue)) != (ssize_t)sizeof(factorValue)) {
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

    if (hfactor_init(&queue, &guard, &sharedMem) == -1) { // initalize
        return -1;
    }

    close(sharedMem); // closes
    sharedMem = -1;
    sem_close(guard);
    guard = NULL;
    mq_close(queue);
    queue = (mqd_t)-1;

    pid_t *workers = (pid_t *)malloc(num_workers * sizeof(pid_t)); // allocate
    if (!workers) {
        queue = mq_open(QUEUE_NAME, O_RDWR);
        guard = sem_open(SEMAPHORE_NAME, 0);
        sharedMem = shm_open(SHM_NAME, O_RDWR, 0666);
        hfactor_deinit(queue, guard, sharedMem);
        return -1;
    }

    for (uint64_t i = 0; i < num_workers; i++) { //create worker
        workers[i] = fork();
        if (workers[i] == -1) {
            for (uint64_t j = 0; j < i; j++) {
                if (workers[j] > 0) 
                kill(workers[j], SIGTERM);
            }
            free(workers);
            queue = mq_open(QUEUE_NAME, O_RDWR);
            guard = sem_open(SEMAPHORE_NAME, 0);
            sharedMem = shm_open(SHM_NAME, O_RDWR, 0666);
            hfactor_deinit(queue, guard, sharedMem);
            return -1;
        }
        if (workers[i] == 0) {
            int childResult = hfactor_process(target);

            if (childResult == 0) {
                exit(0);
            }
            else {
                exit(1);
            }
        }
    }

    queue = mq_open(QUEUE_NAME, O_WRONLY);
    if (queue == (mqd_t)-1) {
        for (uint64_t i = 0; i < num_workers; i++) {
            if (workers[i] > 0)
            kill(workers[i], SIGTERM);
        }
        free(workers);
        guard = sem_open(SEMAPHORE_NAME, 0);
        sharedMem = shm_open(SHM_NAME, O_RDWR, 0666);
        hfactor_deinit(queue, guard, sharedMem);
        return -1;
    }

    for (uint64_t test = 2; test <= (target / 2); test++ ) {
        if (mq_send(queue, (const char*)&test, sizeof(test), 0) == -1) {
            break;
        }
    }

    uint64_t sentinel = 0;
    for (uint64_t i = 0; i < num_workers; i++) {
        if (mq_send(queue, (const char*)&sentinel, sizeof(sentinel), 0) == -1) {
            perror("sentinel");
        }
    }

    mq_close(queue);

    for (uint64_t i = 0; i < num_workers; i++) {
        int status = 0;
        (void)waitpid(workers[i], &status, 0);
    }
    free(workers);

    sharedMem = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (sharedMem == -1) {
        queue = mq_open(QUEUE_NAME, O_RDWR);
        guard = sem_open(SEMAPHORE_NAME, 0);
        hfactor_deinit(queue, guard, sharedMem);
        return -1;
    }

    if (hfactor_report(reporter, sharedMem) == -1) {
        close(sharedMem);
        queue = mq_open(QUEUE_NAME, O_RDWR);
        guard = sem_open(SEMAPHORE_NAME, 0);
        hfactor_deinit(queue, guard, sharedMem);
        return -1;
    }


    queue = mq_open(QUEUE_NAME, O_RDWR);
    guard = sem_open(SEMAPHORE_NAME, 0);
    hfactor_deinit(queue, guard, sharedMem);
    return 0;
}
