#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include "work.h"

// Compiled with:
// gcc -std=gnu11 -Wall main.c -lpthread -ldl

int main(int argc, char *argv[])
{

    int i;
    void *handle;
    pthread_t *tids;
    WORK *work;
    int pthread_count = 0;
    int capacity = 10;
    if (argc < 2) {
        printf("Usage: %s <worker library>\n", argv[0]);
        return 1;
    }

    tids = malloc(capacity * sizeof(pthread_t));
    work = malloc(capacity * sizeof(WORK));

    char *dso = argv[1];
    char dso_full[256];

    snprintf(dso_full, sizeof(dso_full), "./%s", dso);

    handle = dlopen(dso_full, RTLD_LAZY);
    if(handle == NULL){
        return 1;
    }

    WORKER_FUNC worker = (WORKER_FUNC)dlsym(handle, "worker");
    REPORT_FUNC report = (REPORT_FUNC)dlsym(handle, "report");

    while(scanf("%d %d %d", &work[pthread_count].param[0], &work[pthread_count].param[1], &work[pthread_count].param[2]) == 3){
        pthread_count += 1;
        if (pthread_count >= capacity){
            capacity *= 2;
            work = realloc(work, capacity * sizeof(WORK));
            tids = realloc(tids, capacity * sizeof(pthread_t));
        }
    }

    for (i = 0; i < pthread_count; i+=1){
        pthread_create(tids + i, NULL, worker, &work[i]);
    }
    for (i = 0; i < pthread_count; i+=1){
        pthread_join(tids[i], NULL); 
        report(&work[i]);
    }

    free(work);
    free(tids);
    dlclose(handle);

    return 0;
}