#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Do not change the worker's behavior, but its prototype must match pthreads
void *worker(void *arg);

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
        // Set an initial value if your worker expects it
        data[i] = i;
        if (pthread_create(&tids[i], NULL, worker, &data[i]) != 0) {
            perror("pthread_create");
            num_threads = i; // only join threads that started
            break;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(tids[i], NULL);
    }

    for (int i = 0; i < num_threads; i++) {
        printf("%d\n", data[i]);
    }

    free(tids);
    free(data);
    return 0;
}
