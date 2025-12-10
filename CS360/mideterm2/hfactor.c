#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <string.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "hfactor.h"

/*
 * hfactor.c
 *
 * This program demonstrates several inter-process communication (IPC)
 * techniques on POSIX systems: message queues, semaphores, shared memory,
 * pipes, and fork/exec. The goal is to distribute the work of testing
 * whether numbers are factors of a target value across multiple worker
 * processes and collect the found factors.
 *
 * Below are plain-language comments explaining the important concepts
 * and what each function does.
 */

// tests if numbers are factors
int hfactor_process(uint64_t target)
{
    /*
     * Concepts used here:
     * - Message queue (mqueue): a named OS mailbox processes use to send
     *   and receive fixed-size messages. Here, the parent sends candidate
     *   numbers to workers via a queue.
     * - Semaphore: a small kernel object used for coordination. We use a
     *   named semaphore to make updates to the shared memory atomic so
     *   two workers don't overwrite each other's changes.
     * - Shared memory (shm): a named memory object that multiple
     *   processes can open and read/write directly. Here we store the
     *   number of factors found and then the list of factors after that.
     */
    mqd_t mqueue; 
    sem_t *sem;
    int shm;
    uint64_t candidate;

    // open message queue (receive-only). The queue name is defined in
    // hfactor.h as QUEUE_NAME. mq_open returns a descriptor you can use
    // to receive messages. If it fails we return an error.
    mqueue = mq_open(QUEUE_NAME, O_RDONLY);
    if (mqueue == (mqd_t)-1) {
        return -1;
    }

    // open semaphore by name. sem_open returns a pointer to the semaphore
    // object; sem_wait/sem_post are used to lock/unlock around shared
    // memory updates. If open fails, clean up and return error.
    sem = sem_open(SEMAPHORE_NAME, 0);
    if (sem == SEM_FAILED) {
        mq_close(mqueue);
        return -1;
    }

    // open shared memory object (created by the initializer). We open it
    // for read/write so we can update the count and append factors.
    shm = shm_open(SHM_NAME, O_RDWR, 0666);
    if (shm == -1) {
        sem_close(sem);
        mq_close(mqueue);
        return -1;
    }

    // process messages from the queue in a loop. Each message is a
    // uint64_t candidate value. The parent sends a candidate list and
    // then sends a sentinel value (0) to tell workers to stop.
    for (;;) {
        ssize_t received = mq_receive(mqueue, (char*)&candidate, sizeof(candidate), NULL);
        if (received == -1) {
            break; // error or queue closed
        }
        if (candidate == 0) {
            break; // sentinel to stop
        }

        // test if candidate is factor of target
        if (target % candidate == 0) {
            /*
             * lock the semaphore so only one worker at a time modifies
             * the shared memory region. This prevents race conditions
             * where two processes read the old count and then write
             * conflicting updates.
             */
            sem_wait(sem);
            uint64_t num_factors;
            /* The shared memory layout is simple:
             *  - At offset 0: a uint64_t count of how many factors are stored
             *  - At offset 8: the first factor (uint64_t), then subsequent
             *    factors follow (8 bytes each). So factor i is at offset
             *    8 + 8*i.
             * We use lseek/read/write on the shared-memory FD to access it.
             */
            lseek(shm, 0, SEEK_SET);
            read(shm, &num_factors, sizeof(num_factors));

            /*
             * ftruncate changes the size of the shared memory object.
             * Here we increase its size to make room for the new factor.
             * Think of ftruncate as resizing a file: if you make it
             * larger the kernel provides additional space you can write to.
             */
            ftruncate(shm, sizeof(uint64_t) * (num_factors + 2));

            // add factor to end of list
            lseek(shm, 8 + 8 * num_factors, SEEK_SET);
            write(shm, &candidate, sizeof(candidate));

            // update the count at the front
            num_factors++;
            lseek(shm, 0, SEEK_SET);
            write(shm, &num_factors, sizeof(num_factors));

            sem_post(sem);
        }
    }
    close(shm);
    sem_close(sem);
    mq_close(mqueue);

    return 0;
}

// initializes message queue, semaphore, and shared memory
int hfactor_init(mqd_t *q, sem_t **sem, int *shm)
{
    struct mq_attr attr;

    /*
     * Before creating resources, unlink (remove) any existing named
     * objects with the same names. This avoids conflicts from previous
     * runs. mq_unlink/sem_unlink/shm_unlink remove the name from the
     * system; existing open descriptors remain valid until closed.
     */
    mq_unlink(QUEUE_NAME);
    sem_unlink(SEMAPHORE_NAME);
    shm_unlink(SHM_NAME);

    // configure attributes for the message queue (max messages and size)
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(uint64_t);
    attr.mq_curmsgs = 0;

    // create message queue (read/write). The queue will be used by the
    // parent to send candidate numbers and by workers to receive them.
    *q = mq_open(QUEUE_NAME, O_CREAT | O_RDWR, 0666, &attr);
    if (*q == (mqd_t)-1) {
        return -1;
    }

    // create named semaphore with initial value 1 (acts like a mutex)
    *sem = sem_open(SEMAPHORE_NAME, O_CREAT, 0666, 1);
    if (*sem == SEM_FAILED) {
        mq_close(*q);
        mq_unlink(QUEUE_NAME);
        return -1;
    }

    // create shared memory object; we will write a uint64_t count at
    // the start and then append factors after that.
    *shm = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (*shm == -1) {
        sem_close(*sem);
        sem_unlink(SEMAPHORE_NAME);
        mq_close(*q);
        mq_unlink(QUEUE_NAME);
        return -1;
    }

    /* set initial size to hold a single uint64_t (the factor count)
     * and initialize it to zero. write() here writes into the shared
     * memory object via the file descriptor; processes can later grow
     * and read it using lseek/read/write.
     */
    ftruncate(*shm, sizeof(uint64_t));
    uint64_t starting_factors = 0;
    write(*shm, &starting_factors, sizeof(starting_factors));

    return 0;
}

// closes and removes all IPC resources
void hfactor_deinit(mqd_t q, sem_t *sem, int shm)
{
    // close descriptors/handles
    close(shm);
    sem_close(sem);
    mq_close(q);

    /* Unlink the named objects from the system. This is like deleting
     * the files that represent the shared memory, semaphore, and
     * message queue. After unlinking, new calls to mq_open/shm_open
     * with O_CREAT will create fresh objects.
     */
    shm_unlink(SHM_NAME);
    sem_unlink(SEMAPHORE_NAME);
    mq_unlink(QUEUE_NAME);
}

// sends found factors to reporter
int hfactor_report(const char *reporter, int shm)
{
    int pipe_fds[2];
    pid_t child;

    /*
     * Pipes are another IPC primitive: a unidirectional channel created
     * with pipe(). The parent writes to pipe_fds[1] and the child reads
     * from pipe_fds[0]. Here we fork a child that will exec the reporter
     * program; by moving the read end of the pipe to file descriptor 0
     * (stdin) in the child and then exec-ing the reporter, the reporter
     * will read the data we send through the pipe as if it came from
     * standard input.
     */
    // create pipe
    if (pipe(pipe_fds) == -1) {
        return -1;
    }

    // fork new process
    child = fork();
    if (child == -1) {
        close(pipe_fds[0]);
        close(pipe_fds[1]);
        return -1;
    }

    // if in child process...
    if (child == 0) {
        // close the write end; child only needs to read
        close(pipe_fds[1]);
        // free fd 0 (stdin) so we can duplicate the pipe read end into it
        close(0); // free fd 0

        /* fcntl with F_DUPFD duplicates the pipe read end to the lowest
         * available file descriptor >= 0; after closing fd 0 this makes
         * the pipe become the child's stdin (fd 0). This is how we feed
         * the reporter program the data from the parent.
         */
        fcntl(pipe_fds[0], F_DUPFD, 0); // move pipe_fds[0] to fd 0
        close(pipe_fds[0]);
        execlp(reporter, reporter, NULL); // child runs the reporter program
        exit(1);
    }
    // in parent process...
    else {
        // parent writes to the pipe
        close(pipe_fds[0]);

        // how many factors from shared memory
        uint64_t num_factors;
        lseek(shm, 0, SEEK_SET);
        read(shm, &num_factors, sizeof(num_factors));

        // write count to pipe
        write(pipe_fds[1], &num_factors, sizeof(num_factors));

        // write all factors to pipe
        for (uint64_t factor_index = 0; factor_index < num_factors; factor_index++) {
            uint64_t current;
            lseek(shm, 8 + 8 * factor_index, SEEK_SET);
            read(shm, &current, sizeof(current));

            write(pipe_fds[1], &current, sizeof(current)); // send factor through pipe
        }
        close(pipe_fds[1]);
        waitpid(child, NULL, 0);
    }

    return 0;
}

// caluculates all factors of a number
int hfactor_calculate(const char *reporter, uint64_t target, uint64_t num_workers)
{
    mqd_t mqueue;
    sem_t *sem;
    int shm;
    pid_t *worker_pids;

    // intitialize queue, semaphore, and shared memory
    if (hfactor_init(&mqueue, &sem, &shm) == -1) {
        return -1;
    }

    // allocate memory for child PIDs so we can wait for them later
    worker_pids = malloc(sizeof(pid_t) * num_workers);
    if (worker_pids == NULL) {
        hfactor_deinit(mqueue, sem, shm);
        return -1;
    }

    // create worker processes
    for (uint64_t worker_index = 0; worker_index < num_workers; worker_index++) {
        // fork new child process
        worker_pids[worker_index] = fork();

        if (worker_pids[worker_index] == -1) {
            /* If fork fails, send the sentinel value (0) to already
             * created workers so they stop, wait for them to finish,
             * clean up, and return error.
             */
            for (uint64_t free_index = 0; free_index < worker_index; free_index++) {
                uint64_t quit_signal = 0;
                mq_send(mqueue, (char*)&quit_signal, sizeof(quit_signal), 0);
                waitpid(worker_pids[free_index], NULL, 0);
            }
            free(worker_pids);
            hfactor_deinit(mqueue, sem, shm);
            return -1;
        }

        // if in child process
        if (worker_pids[worker_index] == 0) {
            /* Child executes the worker function which will open the
             * same message queue, semaphore, and shared memory by name
             * and then process candidates until it receives the sentinel
             * value 0. After finishing it exits.
             */
            free(worker_pids);
            hfactor_process(target);
            exit(0);
        }
    }

    // parent process: send all candidate divisors to the queue so workers
    // can test them in parallel. We test from 2 up to target/2 (simple
    // but not optimized approach).
    for (uint64_t candidate = 2; candidate <= target / 2; candidate++) {
        mq_send(mqueue, (char*)&candidate, sizeof(candidate), 0); // send all potential factors to message queue and test
    }

    // send sentinel value, 0, to all workers to tell them to stop
    uint64_t quit_signal = 0;
    for (uint64_t worker_index = 0; worker_index < num_workers; worker_index++) {
        mq_send(mqueue, (char*)&quit_signal, sizeof(quit_signal), 0);
    }

    // wait for worker processes to finish
    for (uint64_t worker_index = 0; worker_index < num_workers; worker_index++) {
        waitpid(worker_pids[worker_index], NULL, 0);
    }

    free(worker_pids);

    // send factors to reporter program using a pipe and a fork/exec
    if (hfactor_report(reporter, shm) == -1) {
        hfactor_deinit(mqueue, sem, shm);
        return -1;
    }

    // clean up all IPC resources
    hfactor_deinit(mqueue, sem, shm);

    return 0;
}
