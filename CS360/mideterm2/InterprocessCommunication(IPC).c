/*
 * InterprocessCommunication(IPC).c
 *
 * Simple example program that receives small data messages from a POSIX
 * message queue, computes a small result for each message, and accumulates
 * the results into a shared memory double. A semaphore is used to protect
 * the shared memory update so two processes won't corrupt the value when
 * they write at the same time.
 *
 * This file contains plain-language comments to help newcomers understand
 * the key operating system primitives used here: message queues, semaphores,
 * and shared memory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>    /* POSIX message queues */
#include <semaphore.h> /* POSIX named semaphores */
#include <sys/mman.h>  /* shared memory (mmap) */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* Small struct describing each message's payload.
 * Think of this as the packet the sender puts on the queue. It contains
 * three double values; the receiver multiplies x and y and adds z.
 */
typedef struct {
    double x;
    double y;
    double z;
} DATA;

/* Helper: print an error message and exit. Keeps the example code shorter. */
static void fail(const char *msg) { perror(msg); exit(1); }

int main(int argc, char *argv[])
{
    /* Program expects three command-line arguments:
     *  argv[1] - name of the POSIX message queue to read from
     *  argv[2] - name of the named semaphore used to protect the shared sum
     *  argv[3] - name of the POSIX shared memory object (backing a double)
     *
     * A simple sentinel message { -1, -1, -1 } tells the receiver to stop.
     */
    if (argc < 4) {
        printf("Usage: %s <mqueue> <sem> <shm>\n", argv[0]);
        return 1;
    }

    const char *queue_name = argv[1];
    const char *sem_name   = argv[2];
    const char *mem_name   = argv[3];

    /* special message used to tell this program to exit cleanly */
    const DATA stop = { -1, -1, -1 };

    /* Open the existing message queue for reading. Message queues let
     * separate processes send fixed-size messages to each other without
     * needing a direct pipe or socket connection.
     */
    mqd_t queue = mq_open(queue_name, O_RDONLY);
    if (queue == (mqd_t)-1) fail("mq_open");

    /* Open a named semaphore. A semaphore is a small kernel object that
     * can be used to coordinate access to shared resources. Here it acts
     * like a single lock (mutex) to make sure only one process updates the
     * shared sum at a time.
     */
    sem_t *lock = sem_open(sem_name, 0);
    if (lock == SEM_FAILED) fail("sem_open");

    /* Open the POSIX shared memory object. Shared memory is simply a region
     * of memory that multiple processes can map into their address spaces.
     * Here we expect the shared object to contain one double that is used
     * as an accumulator for results computed from messages.
     */
    int mem_fd = shm_open(mem_name, O_RDWR, 0);
    if (mem_fd < 0) fail("shm_open");

    /* Map the shared memory into our address space so we can read and
     * write the double value directly as if it were a regular variable.
     */
    void *mem = mmap(NULL, sizeof(double), PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, 0);
    if (mem == MAP_FAILED) fail("mmap");
    double *sum = (double *)mem;

    /* Ask the queue about its attributes (like max message size) so we
     * can allocate a safe receive buffer. mq_msgsize is the maximum number
     * of bytes a single message can contain.
     */
    struct mq_attr qinfo;
    if (mq_getattr(queue, &qinfo) == -1) fail("mq_getattr");
    if ((size_t)qinfo.mq_msgsize < sizeof(DATA)) {
        fprintf(stderr, "queue msgsize too small\n");
        return 1;
    }
    char *buf = malloc((size_t)qinfo.mq_msgsize);
    if (!buf) fail("malloc");

    /* Main receive loop: wait for messages, compute a small function of
     * the message data, and add that to the shared sum. The critical
     * section around the update is protected with sem_wait/sem_post so that
     * concurrent writers do not overwrite each other.
     */
    for (;;) {
        unsigned int prio = 0; /* message priority (unused in this example) */
        ssize_t n = mq_receive(queue, buf, (size_t)qinfo.mq_msgsize, &prio);
        if (n < 0) fail("mq_receive");

        /* Reinterpret the received bytes as a DATA struct. This works
         * because the sender is expected to have placed the same struct
         * layout on the queue.
         */
        DATA *msg = (DATA *)buf;

        /* Check for the sentinel that tells us to stop */
        if (msg->x == stop.x && msg->y == stop.y && msg->z == stop.z) break;

        /* Example computation using the values from the message */
        double result = msg->x * msg->y + msg->z;

        /* Lock the semaphore (decrement). If another process holds the
         * semaphore it will block here until the other process calls
         * sem_post(). This makes the update to *sum atomic from our
         * viewpoint.
         */
        if (sem_wait(lock) == -1) fail("sem_wait");
        double now = *sum;
        now += result;
        *sum = now;
        /* Release the semaphore so other processes can update the sum. */
        if (sem_post(lock) == -1) fail("sem_post");
    }

    /* Clean up resources */
    free(buf);
    munmap(mem, sizeof(double));
    close(mem_fd);
    sem_close(lock);
    mq_close(queue);
    return 0;
}
