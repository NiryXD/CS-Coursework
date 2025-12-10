/*
 * Prac2.c
 *
 * Simple word-count coordinator program. It demonstrates common POSIX
 * inter-process communication (IPC) techniques: message queues, named
 * semaphores, and shared memory. The program spawns worker processes
 * (which are separate executables named "worker"), sends them file
 * names via a message queue, and collects per-file word counts in a
 * shared-memory linked list protected by a semaphore.
 *
 * The comments below explain each step in plain English and cover
 * concepts like exec (how a child process replaces itself with a new
 * program), sentinel messages (how workers are told to stop), and
 * why we need semaphores for shared memory updates.
 */

#include "wordcount.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>
#include <semaphore.h>
#include <signal.h>

/* Helper to remove named IPC objects. Called at start and end to avoid
 * leaving stale resources in the system namespace.
 */
static void cleanup_ipc(const char *queue_name, const char *sem_name, const char *shm_name) {
    mq_unlink(queue_name);
    sem_unlink(sem_name);
    shm_unlink(shm_name);
}

int main(int argc, char *argv[]) {
    /* 0: program, 1: num_workers, 2: mqueue name, 3: sem name, 4: shm name,
     * 5...: filenames to process
     */
    if (argc < 6) {
        fprintf(stderr, "Usage: %s <num_workers> <mqueue> <sem> <shm> <file1> [file2...]\n", argv[0]);
        return 1;
    }

    /* Parse number of worker processes (forked children that run the
     * separate 'worker' executable). We limit to a reasonable maximum.
     */
    int num_workers = atoi(argv[1]);
    if (num_workers <= 0 || num_workers > 100) {
        fprintf(stderr, "Invalid number of workers: %d\n", num_workers);
        return 1;
    }

    const char *queue_name = argv[2];
    const char *sem_name = argv[3];
    const char *shm_name = argv[4];
    int num_files = argc - 5;

    /* Remove any previously created IPC objects with the same names so
     * we start from a clean state. This prevents errors when a previous
     * run crashed and left the queue/sem/shm in place.
     */
    cleanup_ipc(queue_name, sem_name, shm_name);

    /* 1) Create a POSIX message queue. This is a system-managed named
     * mailbox that the parent uses to send WorkItem messages (file
     * names) to workers. mq_open with O_CREAT creates the queue.
     */
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = sizeof(WorkItem);
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open(queue_name, O_CREAT | O_WRONLY, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        return 1;
    }

    /* 2) Create a named semaphore. This acts like a mutex that workers
     * use to protect updates to the shared-memory linked list so two
     * workers don't write at the same time and corrupt data.
     */
    sem_t *sem = sem_open(sem_name, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        mq_close(mq);
        cleanup_ipc(queue_name, sem_name, shm_name);
        return 1;
    }

    /* 3) Create a POSIX shared memory object. The parent initializes a
     * small header (ShmHeader) that points to a linked list of Result
     * records appended by workers. We mmap the object to write the
     * header, then unmap and close it; workers will open it by name.
     */
    int shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        sem_close(sem);
        mq_close(mq);
        cleanup_ipc(queue_name, sem_name, shm_name);
        return 1;
    }

    /* Set the object's size to hold the header and initialize fields */
    size_t initial_size = sizeof(ShmHeader);
    if (ftruncate(shm_fd, initial_size) == -1) {
        perror("ftruncate");
        close(shm_fd);
        sem_close(sem);
        mq_close(mq);
        cleanup_ipc(queue_name, sem_name, shm_name);
        return 1;
    }

    void *shm_base = mmap(NULL, initial_size, PROT_READ | PROT_WRITE,
                          MAP_SHARED, shm_fd, 0);
    if (shm_base == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        sem_close(sem);
        mq_close(mq);
        cleanup_ipc(queue_name, sem_name, shm_name);
        return 1;
    }

    /* Initialize linked-list head as empty */
    ShmHeader *header = (ShmHeader *)shm_base;
    header->head_offset = 0;  /* 0 means no results yet */
    header->total_bytes = initial_size;

    munmap(shm_base, initial_size);
    close(shm_fd);

    /* 4) Fork worker processes. Each worker is a separate process that
     * will call execl to replace its image with the './worker'
     * executable. Typical pattern: fork() creates a child process; the
     * child then calls exec*() to become the worker program. The parent
     * keeps the child's PID so it can wait for it later.
     */
    pid_t *pids = malloc(sizeof(pid_t) * num_workers);
    if (!pids) {
        perror("malloc");
        sem_close(sem);
        mq_close(mq);
        cleanup_ipc(queue_name, sem_name, shm_name);
        return 1;
    }

    for (int i = 0; i < num_workers; i++) {
        pids[i] = fork();
        if (pids[i] == -1) {
            perror("fork");
            /* If fork fails, terminate already-started workers and exit */
            for (int j = 0; j < i; j++) {
                kill(pids[j], SIGTERM);
                waitpid(pids[j], NULL, 0);
            }
            free(pids);
            sem_close(sem);
            mq_close(mq);
            cleanup_ipc(queue_name, sem_name, shm_name);
            return 1;
        }

        if (pids[i] == 0) {
            /* Child process: close parent's handles and exec the worker
             * executable. execl replaces the current process image with
             * the program './worker' and passes the IPC names as args.
             * If execl returns, an error occurred.
             */
            free(pids);
            sem_close(sem);
            mq_close(mq);
            execl("./worker", "worker", queue_name, sem_name, shm_name, NULL);
            perror("execl");
            exit(1);
        }
    }

    /* 5) Send filenames to the message queue as WorkItem messages. The
     * workers will receive these and process each file.
     */
    for (int i = 5; i < argc; i++) {
        WorkItem work;
        memset(&work, 0, sizeof(work));
        strncpy(work.filename, argv[i], sizeof(work.filename) - 1);
        
        if (mq_send(mq, (char *)&work, sizeof(work), 0) == -1) {
            perror("mq_send");
        }
    }

    /* 6) Send sentinel messages to tell workers to stop. We send an
     * empty filename as a sentinel; when a worker reads this it knows
     * there is no more work and it should exit.
     */
    WorkItem stop;
    memset(&stop, 0, sizeof(stop));
    stop.filename[0] = '\0';

    for (int i = 0; i < num_workers; i++) {
        if (mq_send(mq, (char *)&stop, sizeof(stop), 0) == -1) {
            perror("mq_send sentinel");
        }
    }

    /* 7) Wait for all worker processes to finish and report any errors. */
    for (int i = 0; i < num_workers; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            fprintf(stderr, "Worker %d exited with error code %d\n", i, WEXITSTATUS(status));
        }
    }

    free(pids);

    /* 8) Read and print results from shared memory. We open the shared
     * memory object read-only, mmap it, then walk the linked list of
     * Result records that workers appended under semaphore protection.
     */
    shm_fd = shm_open(shm_name, O_RDONLY, 0);
    if (shm_fd == -1) {
        perror("shm_open for reading");
        sem_close(sem);
        mq_close(mq);
        cleanup_ipc(queue_name, sem_name, shm_name);
        return 1;
    }

    struct stat shm_stat;
    if (fstat(shm_fd, &shm_stat) == -1) {
        perror("fstat");
        close(shm_fd);
        sem_close(sem);
        mq_close(mq);
        cleanup_ipc(queue_name, sem_name, shm_name);
        return 1;
    }

    shm_base = mmap(NULL, shm_stat.st_size, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_base == MAP_FAILED) {
        perror("mmap for reading");
        close(shm_fd);
        sem_close(sem);
        mq_close(mq);
        cleanup_ipc(queue_name, sem_name, shm_name);
        return 1;
    }

    header = (ShmHeader *)shm_base;
    size_t total_words = 0;

    printf("\n=== Word Count Results ===\n");
    
    /* Traverse the linked list starting at header->head_offset and print results */
    size_t current_offset = header->head_offset;
    while (current_offset != 0) {
        Result *result = (Result *)((char *)shm_base + current_offset);
        printf("%s: %zu words\n", result->filename, result->word_count);
        total_words += result->word_count;
        current_offset = result->next_offset;
    }

    printf("\nTotal: %zu words across %d files\n", total_words, num_files);

    /* 9) Cleanup resources: unmap/close shared memory and unlink IPC names */
    munmap(shm_base, shm_stat.st_size);
    close(shm_fd);
    sem_close(sem);
    mq_close(mq);
    cleanup_ipc(queue_name, sem_name, shm_name);

    return 0;
}

/* 
#ifndef WORDCOUNT_H
#define WORDCOUNT_H

#include <stddef.h>

// Message sent through queue to workers
typedef struct {
    char filename[256];
} WorkItem;

// Result node stored in shared memory
// NOTE: 'next' is stored as an OFFSET from shm base, not a real pointer
typedef struct {
    char filename[256];
    size_t word_count;
    size_t next_offset;  // Offset to next Result, or 0 if NULL
} Result;

// Shared memory header (at offset 0)
typedef struct {
    size_t head_offset;   // Offset to first Result, or 0 if empty
    size_t total_bytes;   // Current size of shared memory
} ShmHeader;

// Function to count words in a file
size_t count_words(const char *filename);

#endif
*/