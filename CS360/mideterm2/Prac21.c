/*
 * Prac21.c  (worker)
 *
 * This is the worker program for a simple word-count system. The
 * parent process sends file names over a POSIX message queue. Each
 * worker receives file names, counts words in each file, and appends
 * a Result record into a shared-memory linked list. A named semaphore
 * is used as a mutex so multiple workers don't write the list at the
 * same time.
 *
 * The goal of the comments below is to explain the non-trivial ideas
 * in plain English: how the message queue passes work, how a
 * semaphore protects shared memory updates, why the shared memory is
 * grown with ftruncate and remapped, and the sentinel pattern used to
 * tell workers when to stop.
 */

#include "wordcount.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>
#include <semaphore.h>

/*
 * count_words
 * A simple utility that opens a file and counts words by scanning
 * characters. We treat any run of non-whitespace characters as a word.
 * This function is intentionally straightforward so it's easy to
 * reason about the worker's main responsibility.
 */
size_t count_words(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        /* If the file can't be opened, report the error and return 0 */
        perror(filename);
        return 0;
    }

    size_t count = 0;
    int in_word = 0;
    int c;

    while ((c = fgetc(f)) != EOF) {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            in_word = 0; /* we're in whitespace */
        } else {
            /* first non-whitespace after whitespace starts a new word */
            if (!in_word) {
                count++;
                in_word = 1;
            }
        }
    }

    fclose(f);
    return count;
}

/*
 * main
 * Expected args: <program> <mqueue> <sem> <shm>
 * The worker does NOT create the IPC objects; it only opens them by
 * name (the parent/driver creates them). This keeps responsibilities
 * separate: the parent sets up shared state, and workers simply use it.
 */
int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Worker usage: %s <mqueue> <sem> <shm>\n", argv[0]);
        return 1;
    }

    const char *queue_name = argv[1];
    const char *sem_name = argv[2];
    const char *shm_name = argv[3];

    /*
     * Open the message queue for reading. The queue acts like a
     * thread-safe mailbox the parent uses to hand out filenames. We
     * open read-only because workers only consume messages.
     */
    mqd_t mq = mq_open(queue_name, O_RDONLY);
    if (mq == (mqd_t)-1) {
        perror("worker: mq_open");
        return 1;
    }

    /*
     * Open the named semaphore. This is a system-wide semaphore used
     * to serialize modifications to shared memory. Think of it as a
     * lock that all workers must acquire before changing the shared
     * linked list.
     */
    sem_t *sem = sem_open(sem_name, 0);
    if (sem == SEM_FAILED) {
        perror("worker: sem_open");
        mq_close(mq);
        return 1;
    }

    /*
     * Open the shared memory object (created by the parent). We open
     * it read/write because we'll map it and append data.
     */
    int shm_fd = shm_open(shm_name, O_RDWR, 0);
    if (shm_fd == -1) {
        perror("worker: shm_open");
        sem_close(sem);
        mq_close(mq);
        return 1;
    }

    /* Main loop: receive WorkItem messages from the queue and process them */
    WorkItem work;
    while (1) {
        /* mq_receive blocks until a message arrives. Each message
         * contains a filename to process. On error mq_receive returns -1.
         */
        ssize_t received = mq_receive(mq, (char *)&work, sizeof(work), NULL);
        if (received == -1) {
            perror("worker: mq_receive");
            break; /* breaking out will run cleanup */
        }

        /* Sentinel pattern: parent sends an empty filename as a
         * special message meaning "no more work". When we see it we
         * break the loop and exit cleanly.
         */
        if (work.filename[0] == '\0') {
            break;
        }

        /* Do the real work: count words in the file */
        size_t word_count = count_words(work.filename);

        /*
         * Append the result to the shared memory list. Because many
         * workers may try to append concurrently we must hold the
         * semaphore while we read/resize/remap and write the list.
         * The sequence below is:
         *  - sem_wait to acquire lock
         *  - fstat to learn current shm size
         *  - mmap the current region
         *  - compute the new size and unmap
         *  - ftruncate to grow the object
         *  - mmap again with the new size
         *  - write the Result at the end and update header
         *  - munmap and sem_post to release lock
         *
         * Why grow + remap? POSIX shared memory (shm_open) has a fixed
         * size until you call ftruncate. To append we extend the
         * underlying object and then remap it so the new bytes are
         * available in our address space.
         */
        sem_wait(sem);  /* LOCK */

        struct stat shm_stat;
        if (fstat(shm_fd, &shm_stat) == -1) {
            perror("worker: fstat");
            sem_post(sem);
            continue; /* skip this item but keep worker alive */
        }

        /* Map the current shared memory so we can read header fields */
        void *shm_base = mmap(NULL, shm_stat.st_size, PROT_READ | PROT_WRITE,
                              MAP_SHARED, shm_fd, 0);
        if (shm_base == MAP_FAILED) {
            perror("worker: mmap");
            sem_post(sem);
            continue;
        }

        ShmHeader *header = (ShmHeader *)shm_base;
        size_t old_size = header->total_bytes;
        size_t new_offset = old_size; /* append at end */
        size_t new_size = old_size + sizeof(Result);

        /* Unmap the old mapping before changing the object size */
        munmap(shm_base, old_size);

        /* Grow the shared memory object so there is space for the new Result */
        if (ftruncate(shm_fd, new_size) == -1) {
            perror("worker: ftruncate");
            sem_post(sem);
            continue;
        }

        /* Remap with the larger size so we can write into the new area */
        shm_base = mmap(NULL, new_size, PROT_READ | PROT_WRITE,
                        MAP_SHARED, shm_fd, 0);
        if (shm_base == MAP_FAILED) {
            perror("worker: mmap after grow");
            sem_post(sem);
            continue;
        }

        /* Recalculate the header pointer since address changed */
        header = (ShmHeader *)shm_base;

        /* Fill the Result structure at the new_offset (end of previous data) */
        Result *new_result = (Result *)((char *)shm_base + new_offset);
        strncpy(new_result->filename, work.filename, sizeof(new_result->filename) - 1);
        new_result->filename[sizeof(new_result->filename) - 1] = '\0';
        new_result->word_count = word_count;
        /* Link into the list by pointing this node to the old head */
        new_result->next_offset = header->head_offset;

        /* Update the header to point to the new head and record the size */
        header->head_offset = new_offset;  /* new node becomes the head */
        header->total_bytes = new_size;

        /* Unmap the shared memory now that updates are done */
        munmap(shm_base, new_size);

        sem_post(sem);  /* UNLOCK */
    }

    /* Cleanup: close descriptors and semaphore handles */
    close(shm_fd);
    sem_close(sem);
    mq_close(mq);

    return 0;
}