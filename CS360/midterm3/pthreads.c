#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Do not change the worker's behavior, but its prototype must match pthreads
void *worker(void *arg);

/*
 * Quick, plain-English notes about pthreads used in this file:
 * - A "pthread" is a lightweight thread of execution inside the program.
 * - `pthread_create` starts a new thread. It needs:
 *     1) a place to store the thread id (`pthread_t`),
 *     2) optional attributes (NULL for defaults),
 *     3) the function the thread will run (here: `worker`),
 *     4) a single `void *` argument to pass to that function.
 * - The worker function must match the prototype `void *fun(void *)` so
 *   the pthread API can call it. Use the argument to give each thread
 *   whatever small bit of state it needs (here: a pointer into `data`).
 * - `pthread_join` waits for a specific thread to finish. This is how the
 *   main thread ensures all worker threads completed before continuing.
 * - Common pitfalls to remember for exams:
 *     * Don't pass the address of a local variable that goes out of scope.
 *     * Be careful about sharing data between threads — protect shared data
 *       with mutexes (not needed in this simple example but important).
 */

int main(int argc, char *argv[])
{
    int num_threads;
    pthread_t *tids;
    int *data;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <num threads>\n", argv[0]);
        return 1;
    }

    if (sscanf(argv[1], "%d", &num_threads) != 1 ||
        num_threads <= 0 || num_threads > 1000) {
        fprintf(stderr, "Invalid number of threads: '%s'\n", argv[1]);
        return 1;
    }

    tids = calloc(num_threads, sizeof(*tids));
    data = calloc(num_threads, sizeof(*data));
    if (!tids || !data) {
        perror("calloc");
        free(tids);
        free(data);
        return 1;
    }

    for (int i = 0; i < num_threads; i++) {
        /*
         * Give each thread its own small piece of input data.
         * Here `data` is an array and we pass a pointer to the i'th
         * element (`&data[i]`). That pointer will remain valid because
         * `data` is allocated on the heap and not a local stack variable.
         * The worker receives that pointer as a `void *` and can cast it
         * back to `int *` to read or modify the element.
         */
        data[i] = i;

        /*
         * Create the thread: store its id in `tids[i]`, use default
         * attributes, run `worker`, and pass `&data[i]` as the argument.
         * If `pthread_create` fails we stop creating more threads and
         * will only `join` those that were started.
         */
        if (pthread_create(&tids[i], NULL, worker, &data[i]) != 0) {
            perror("pthread_create");
            num_threads = i; // only join threads that started
            break;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        /*
         * Wait for thread `tids[i]` to finish. `pthread_join` blocks until
         * the thread has exited. This avoids the main program exiting and
         * reclaiming resources while worker threads are still running.
         */
        pthread_join(tids[i], NULL);
    }

    for (int i = 0; i < num_threads; i++) {
        printf("%d\n", data[i]);
    }

    free(tids);
    free(data);
    return 0;
}
