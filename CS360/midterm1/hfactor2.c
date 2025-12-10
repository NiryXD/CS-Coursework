#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <mqueue.h>
#include <semaphore.h>
#include <errno.h>
#include "hfactor.h"

int hfactor_process(uint64_t target)
{
    mqd_t q;
    sem_t *sem;
    int shm;
    uint64_t candidate;
    ssize_t bytes_read;
    
    // Open the message queue
    q = mq_open(QUEUE_NAME, O_RDONLY);
    if (q == (mqd_t)-1) {
        perror("mq_open in child");
        return -1;
    }
    
    // Open the semaphore
    sem = sem_open(SEMAPHORE_NAME, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open in child");
        mq_close(q);
        return -1;
    }
    
    // Open the shared memory
    shm = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm == -1) {
        perror("shm_open in child");
        mq_close(q);
        sem_close(sem);
        return -1;
    }
    
    // Process messages from the queue
    while (1) {
        bytes_read = mq_receive(q, (char*)&candidate, sizeof(candidate), NULL);
        if (bytes_read < 0) {
            perror("mq_receive");
            break;
        }
        
        // Check for sentinel value (0 means quit)
        if (candidate == 0) {
            break;
        }
        
        // Test if candidate is a factor of target
        if (target % candidate == 0) {
            // Lock the semaphore
            if (sem_wait(sem) == -1) {
                perror("sem_wait");
                break;
            }
            
            // Read current size from shared memory
            uint64_t size;
            lseek(shm, 0, SEEK_SET);
            if (read(shm, &size, sizeof(size)) != sizeof(size)) {
                perror("read size");
                sem_post(sem);
                break;
            }
            
            // Grow the shared memory
            off_t new_size = sizeof(uint64_t) + (size + 1) * sizeof(uint64_t);
            if (ftruncate(shm, new_size) == -1) {
                perror("ftruncate");
                sem_post(sem);
                break;
            }
            
            // Write the new factor at the end
            lseek(shm, sizeof(uint64_t) + size * sizeof(uint64_t), SEEK_SET);
            if (write(shm, &candidate, sizeof(candidate)) != sizeof(candidate)) {
                perror("write factor");
                sem_post(sem);
                break;
            }
            
            // Update the size
            size++;
            lseek(shm, 0, SEEK_SET);
            if (write(shm, &size, sizeof(size)) != sizeof(size)) {
                perror("write size");
                sem_post(sem);
                break;
            }
            
            // Unlock the semaphore
            if (sem_post(sem) == -1) {
                perror("sem_post");
                break;
            }
        }
    }
    
    // Clean up (close but don't unlink - parent will do that)
    close(shm);
    sem_close(sem);
    mq_close(q);
    
    return 0;
}

int hfactor_init(mqd_t *q, sem_t **sem, int *shm)
{
    // Create message queue
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(uint64_t);
    attr.mq_curmsgs = 0;
    
    *q = mq_open(QUEUE_NAME, O_CREAT | O_EXCL | O_RDWR, 0666, &attr);
    if (*q == (mqd_t)-1) {
        perror("mq_open");
        return -1;
    }
    
    // Create semaphore
    *sem = sem_open(SEMAPHORE_NAME, O_CREAT | O_EXCL, 0666, 1);
    if (*sem == SEM_FAILED) {
        perror("sem_open");
        mq_close(*q);
        mq_unlink(QUEUE_NAME);
        return -1;
    }
    
    // Create shared memory
    *shm = shm_open(SHM_NAME, O_CREAT | O_EXCL | O_RDWR, 0666);
    if (*shm == -1) {
        perror("shm_open");
        mq_close(*q);
        mq_unlink(QUEUE_NAME);
        sem_close(*sem);
        sem_unlink(SEMAPHORE_NAME);
        return -1;
    }
    
    // Initialize shared memory with size 0
    uint64_t initial_size = 0;
    if (ftruncate(*shm, sizeof(initial_size)) == -1) {
        perror("ftruncate");
        close(*shm);
        shm_unlink(SHM_NAME);
        mq_close(*q);
        mq_unlink(QUEUE_NAME);
        sem_close(*sem);
        sem_unlink(SEMAPHORE_NAME);
        return -1;
    }
    
    if (write(*shm, &initial_size, sizeof(initial_size)) != sizeof(initial_size)) {
        perror("write initial size");
        close(*shm);
        shm_unlink(SHM_NAME);
        mq_close(*q);
        mq_unlink(QUEUE_NAME);
        sem_close(*sem);
        sem_unlink(SEMAPHORE_NAME);
        return -1;
    }
    
    return 0;
}

void hfactor_deinit(mqd_t q, sem_t *sem, int shm)
{
    // Close all resources
    mq_close(q);
    sem_close(sem);
    close(shm);
    
    // Unlink all resources
    mq_unlink(QUEUE_NAME);
    sem_unlink(SEMAPHORE_NAME);
    shm_unlink(SHM_NAME);
}

int hfactor_report(const char *reporter, int shm)
{
    int pipefd[2];
    pid_t pid;
    
    // Create pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }
    
    // Fork child process
    pid = fork();
    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }
    
    if (pid == 0) {
        // Child process
        // Close write end
        close(pipefd[1]);
        
        // Redirect stdin to pipe read end
        if (dup2(pipefd[0], STDIN_FILENO) == -1) {
            perror("dup2");
            exit(1);
        }
        close(pipefd[0]);
        
        // Execute reporter program
        execl(reporter, reporter, NULL);
        perror("execl");
        exit(1);
    }
    
    // Parent process
    // Close read end
    close(pipefd[0]);
    
    // Read size from shared memory
    uint64_t size;
    lseek(shm, 0, SEEK_SET);
    if (read(shm, &size, sizeof(size)) != sizeof(size)) {
        perror("read size");
        close(pipefd[1]);
        return -1;
    }
    
    // Write size to pipe
    if (write(pipefd[1], &size, sizeof(size)) != sizeof(size)) {
        perror("write size to pipe");
        close(pipefd[1]);
        return -1;
    }
    
    // Write all factors to pipe
    for (uint64_t i = 0; i < size; i++) {
        uint64_t factor;
        lseek(shm, sizeof(uint64_t) + i * sizeof(uint64_t), SEEK_SET);
        if (read(shm, &factor, sizeof(factor)) != sizeof(factor)) {
            perror("read factor");
            close(pipefd[1]);
            return -1;
        }
        
        if (write(pipefd[1], &factor, sizeof(factor)) != sizeof(factor)) {
            perror("write factor to pipe");
            close(pipefd[1]);
            return -1;
        }
    }
    
    // Close pipe
    close(pipefd[1]);
    
    // Wait for child to finish
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return -1;
    }
    
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
        return -1;
    }
    
    return 0;
}

int hfactor_calculate(const char *reporter, uint64_t target, uint64_t num_workers)
{
    mqd_t q;
    sem_t *sem;
    int shm;
    pid_t *workers;
    uint64_t i;
    
    // Initialize IPC resources
    if (hfactor_init(&q, &sem, &shm) == -1) {
        return -1;
    }
    
    // Allocate array for worker PIDs
    workers = malloc(num_workers * sizeof(pid_t));
    if (!workers) {
        perror("malloc");
        hfactor_deinit(q, sem, shm);
        return -1;
    }
    
    // Close resources in parent before forking
    // (children will open them in hfactor_process)
    close(shm);
    sem_close(sem);
    mq_close(q);
    
    // Fork worker processes
    for (i = 0; i < num_workers; i++) {
        workers[i] = fork();
        if (workers[i] == -1) {
            perror("fork");
            // Clean up already forked workers
            for (uint64_t j = 0; j < i; j++) {
                kill(workers[j], SIGTERM);
            }
            free(workers);
            // Re-open to clean up
            q = mq_open(QUEUE_NAME, O_RDWR);
            sem = sem_open(SEMAPHORE_NAME, 0);
            shm = shm_open(SHM_NAME, O_RDWR, 0666);
            hfactor_deinit(q, sem, shm);
            return -1;
        }
        
        if (workers[i] == 0) {
            // Child process
            int ret = hfactor_process(target);
            exit(ret);
        }
    }
    
    // Re-open message queue in parent to send messages
    q = mq_open(QUEUE_NAME, O_WRONLY);
    if (q == (mqd_t)-1) {
        perror("mq_open parent");
        for (i = 0; i < num_workers; i++) {
            kill(workers[i], SIGTERM);
        }
        free(workers);
        // Re-open to clean up
        sem = sem_open(SEMAPHORE_NAME, 0);
        shm = shm_open(SHM_NAME, O_RDWR, 0666);
        hfactor_deinit(q, sem, shm);
        return -1;
    }
    
    // Send factor candidates to message queue
    for (uint64_t candidate = 2; candidate <= target / 2; candidate++) {
        if (mq_send(q, (char*)&candidate, sizeof(candidate), 0) == -1) {
            perror("mq_send");
            break;
        }
    }
    
    // Send sentinel value (0) to all workers to signal them to quit
    uint64_t sentinel = 0;
    for (i = 0; i < num_workers; i++) {
        if (mq_send(q, (char*)&sentinel, sizeof(sentinel), 0) == -1) {
            perror("mq_send sentinel");
        }
    }
    
    // Close message queue after sending all messages
    mq_close(q);
    
    // Wait for all workers to finish
    for (i = 0; i < num_workers; i++) {
        int status;
        if (waitpid(workers[i], &status, 0) == -1) {
            perror("waitpid");
        }
    }
    
    free(workers);
    
    // Re-open shared memory for reporting
    shm = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm == -1) {
        perror("shm_open for report");
        // Re-open to clean up
        q = mq_open(QUEUE_NAME, O_RDWR);
        sem = sem_open(SEMAPHORE_NAME, 0);
        hfactor_deinit(q, sem, shm);
        return -1;
    }
    
    // Report the factors
    if (hfactor_report(reporter, shm) == -1) {
        close(shm);
        // Re-open to clean up
        q = mq_open(QUEUE_NAME, O_RDWR);
        sem = sem_open(SEMAPHORE_NAME, 0);
        hfactor_deinit(q, sem, shm);
        return -1;
    }
    
    // Clean up all resources
    q = mq_open(QUEUE_NAME, O_RDWR);
    sem = sem_open(SEMAPHORE_NAME, 0);
    hfactor_deinit(q, sem, shm);
    
    return 0;
}