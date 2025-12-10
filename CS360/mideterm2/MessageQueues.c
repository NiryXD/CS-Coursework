/*
 * MessageQueues.c
 *
 * Simple receiver example using POSIX message queues (mqueue).
 *
 * Layman overview:
 * - A POSIX message queue is like a named mailbox managed by the OS.
 * - Any process that knows the queue's name can open it and send or
 *   receive fixed-size messages.
 * - This program opens a named queue (given on the command line) and
 *   repeatedly reads messages. If a message contains the key "QUIT",
 *   the receiver stops.
 *
 * NOTE: This is a receiver only. You need a separate sender program that
 * opens the same queue name and posts messages of the same size/format.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mqueue.h> /* POSIX message queue API */

/* Small message layout used by both sender and receiver. The sender
 * places these bytes on the queue; the receiver casts the received
 * bytes back into this struct to read the fields.
 */
// RECEIVER
struct Data {
    char   key[32]; /* a short string identifier or command */
    int value;      /* a numeric payload */
};

int main(int argc, char *argv[])
{
    /* Expect the message queue name as a single argument. The name is
     * typically a string like "/myqueue" on POSIX systems.
     */
    if (argc < 2) {
        printf("Usage: %s <mqueue file>\n", argv[0]);
        return 1;
    }

    mqd_t mq;              /* message queue descriptor (like a file handle) */
    struct Data data;      /* buffer for incoming message */
    ssize_t bytes;         /* number of bytes received */
    unsigned int prio;     /* message priority (unused here) */

    /* Open the named message queue for reading. The queue must already
     * exist and have been created by a sender or other setup code.
     * mq_open returns a descriptor that we use with mq_receive.
     */
    mq = mq_open(argv[1], O_RDONLY);

    if (mq < 0) {
        /* Common reasons for failure: the queue name doesn't exist, or
         * permissions prevent opening. On some systems you may need to
         * create the queue first from a separate setup program.
         */
        perror("mq_open");
        return 1;
    }

    /* Main receive loop: keep reading messages until we see the
     * special key "QUIT" or encounter an error. mq_receive will fill
     * our buffer with the message bytes; we then interpret them as
     * a `struct Data` (must match what the sender sent).
     */
    for (;;) {
        bytes = mq_receive(mq, (char*)&data, sizeof(data), &prio);
        if (bytes < 0) {
            /* If receive fails, print an error and stop. A common mistake
             * is to pass a buffer too small; make sure the sender and
             * receiver agree on message size.
             */
            perror("mq_receive");
            break;
        }
        else if (!strcmp(data.key, "QUIT")) {
            /* A conventional way to tell a receiver to stop is to send
             * a special message — here we use the key string "QUIT".
             */
            break;
        }
        /* Otherwise print the received value and key. */
        printf("%d: %s\n", data.value, data.key);
    }

    /* Close the queue descriptor when done. The named queue itself
     * persists until someone unlinks it (similar to removing a file).
     */
    mq_close(mq);
    return 0;
}