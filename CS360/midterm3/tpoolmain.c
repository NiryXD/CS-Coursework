#include <stdio.h>
#include "tpool.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

// This file is NOT graded. Only your `tpool.c` will be tested.
//
// Layman summary:
// - This is a small example program that demonstrates how to use the
//   thread pool implemented in `tpool.c`.
// - It shows three simple use-cases:
//     1) submit work and read results, but keep the pool open;
//     2) submit work and then close the pool after using results;
//     3) use the pool to run file-hashing functions concurrently.
// - You can read these functions to see how the API is intended to be
//   used: `thread_pool_open`, `thread_pool_execute`, and
//   `thread_pool_close`.

static uint64_t executor(int value)
{
    // Simple example "executor" function used by the demo.
    // Given an integer, it returns its square. The thread pool code
    // expects an `Executor` function that takes an `int` and returns
    // a `uint64_t` result.
    return value * value;
}

void threadPoolExecuteFunction() {
    void *handle;
    uint64_t *results;
    int i;
    const int values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const int num_values = sizeof(values) / sizeof(values[0]);

    handle = thread_pool_open(30);
    if (handle == NULL) {
        printf("thread_pool_open: could not initialize thread pool.\n");
        return -1;
    }

    results = thread_pool_execute(handle, values, num_values, executor);
    if (results == NULL) {
        printf("unable to execute on thread pool.\n");
        return -1;
    }
    // Do something with results
    for (i = 0;i < num_values;i+=1) {
        printf("Result %d = %lu\n", i, results[i]);
    }
    // IMPORTANT: The caller (this code) is responsible for freeing the
    // results buffer returned by `thread_pool_execute` when done.
    free(results);

    // Note: this function leaves the thread pool open. That means the
    // worker threads are still running and available for more work.
    // We return here after printing results.
    return 0;
}

void threadPoolCloseFunction() {
    void *handle;
    uint64_t *results;
    int i;
    const int values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    const int num_values = sizeof(values) / sizeof(values[0]);

    handle = thread_pool_open(30);
    if (handle == NULL) {
        printf("thread_pool_open: could not initialize thread pool.\n");
        return -1;
    }

    results = thread_pool_execute(handle, values, num_values, executor);
    if (results == NULL) {
        printf("unable to execute on thread pool.\n");
        thread_pool_close(handle);
        return -1;
    }
    // Do something with results
    for (i = 0;i < num_values;i+=1) {
        printf("Result %d = %lu\n", i, results[i]);
    }
    // Free the results array (we allocated it in thread_pool_execute).
    free(results);

    // Now that we're done with the pool, close it. This tells the pool
    // to stop accepting work, wakes any sleeping workers, and `join`s
    // them (waits for them to finish) before freeing pool memory.
    thread_pool_close(handle);

    return 0;
}

void hashingFunctions() {
    void *handle;
    uint64_t *results;
    int i;
    const int num_values = 2;
    int values[num_values];

    values[0] = open("somefile.txt", O_RDONLY);
    values[1] = open("another.txt", O_RDONLY);

    // We won't need 4 threads for this example, but the execute function
    // still needs to work if not all worker threads are given work.
    handle = thread_pool_open(4);
    if (handle == NULL) {
        printf("thread_pool_open: could not initialize thread pool.\n");
        return -1;
    }

    results = thread_pool_execute(handle, values, num_values, hash32);

    // Close the files.
    close(values[0]);
    close(values[1]);

    if (results != NULL) {
        // Results are ordered.
        // First result is the result of the first integer (somefile.txt)
        printf("Hash of 'somefile.txt' = 0x%08lx\n", results[0]);
        // Second result is the result of the second integer (another.txt)
        printf("Hash of 'another.txt'  = 0x%08lx\n", results[1]);

        // It's our responsibility to free the result memory.
        free(results);
    }
    else {
        printf("Hashing failed.\n");
    }

    // Close, join, and free the thread pool
    thread_pool_close(handle);

    return 0;
}

int main()
{
    // printf("Write your tests here.\n");
    threadPoolExecuteFunction();
    threadPoolCloseFunction();
    hashingFunctions();
    return 0;
}
