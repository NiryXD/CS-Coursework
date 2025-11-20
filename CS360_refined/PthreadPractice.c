#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <dlfcn.h>
#include <stdbool.h>
#include <stdlib.h>

struct Work {
    int test_val;
    int test_target;
    bool result;
};

typedef void *(*WORKERFUNC)(void *arg);
typedef void (*REPORTERFUNC)(const struct Work *w);

int main(int argc, char *argv[])
{
    int target;
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <target> <dso>\n", argv[0]);
        return 1;
    }
    if (1 != sscanf(argv[1], "%d", &target)) {
        fprintf(stderr, "Invalid target '%s'\n", argv[1]);
        return 1;
    }

    // Load the DSO - try multiple variations
    void *handle = NULL;
    char path[512];
    
    // Try original path as-is
    handle = dlopen(argv[2], RTLD_LAZY);
    
    // Try with ./ prefix
    if (!handle) {
        snprintf(path, sizeof(path), "./%s", argv[2]);
        handle = dlopen(path, RTLD_LAZY);
    }
    
    // Try adding .so extension
    if (!handle) {
        snprintf(path, sizeof(path), "%s.so", argv[2]);
        handle = dlopen(path, RTLD_LAZY);
    }
    
    // Try ./ with .so
    if (!handle) {
        snprintf(path, sizeof(path), "./%s.so", argv[2]);
        handle = dlopen(path, RTLD_LAZY);
    }
    
    // Try with lib prefix (only if doesn't already start with lib)
    if (!handle && strncmp(argv[2], "lib", 3) != 0) {
        snprintf(path, sizeof(path), "lib%s", argv[2]);
        handle = dlopen(path, RTLD_LAZY);
    }
    
    // Try ./lib prefix
    if (!handle && strncmp(argv[2], "lib", 3) != 0) {
        snprintf(path, sizeof(path), "./lib%s", argv[2]);
        handle = dlopen(path, RTLD_LAZY);
    }
    
    // Try with lib prefix and .so extension
    if (!handle && strncmp(argv[2], "lib", 3) != 0) {
        snprintf(path, sizeof(path), "lib%s.so", argv[2]);
        handle = dlopen(path, RTLD_LAZY);
    }
    
    // Try ./lib prefix and .so extension
    if (!handle && strncmp(argv[2], "lib", 3) != 0) {
        snprintf(path, sizeof(path), "./lib%s.so", argv[2]);
        handle = dlopen(path, RTLD_LAZY);
    }
    
    if (!handle) {
        fprintf(stderr, "Failed to load DSO: %s\n", dlerror());
        return 1;
    }

    // Extract the two functions from the DSO
    WORKERFUNC dso_worker_func = (WORKERFUNC)dlsym(handle, "dso_worker_func");
    if (!dso_worker_func) {
        fprintf(stderr, "Failed to load dso_worker_func: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }

    REPORTERFUNC dso_reporter_func = (REPORTERFUNC)dlsym(handle, "dso_reporter_func");
    if (!dso_reporter_func) {
        fprintf(stderr, "Failed to load dso_reporter_func: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }

    // Allocate arrays for threads and Work structures
    pthread_t *threads = (pthread_t *)malloc(target * sizeof(pthread_t));
    struct Work *works = (struct Work *)malloc(target * sizeof(struct Work));
    
    if (!threads || !works) {
        fprintf(stderr, "Memory allocation failed\n");
        free(threads);
        free(works);
        dlclose(handle);
        return 1;
    }

    // Spawn target number of threads
    for (int i = 0; i < target; i++) {
        works[i].test_val = i + 1;  // Thread number starting at #1 (not 0!)
        works[i].test_target = target;
        works[i].result = false;
        
        if (pthread_create(&threads[i], NULL, dso_worker_func, &works[i]) != 0) {
            fprintf(stderr, "Failed to create thread %d\n", i);
            // Clean up already created threads
            for (int j = 0; j < i; j++) {
                pthread_join(threads[j], NULL);
            }
            free(threads);
            free(works);
            dlclose(handle);
            return 1;
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < target; i++) {
        pthread_join(threads[i], NULL);
    }

    // Call the reporter function in ascending order (starting with thread 0)
    for (int i = 0; i < target; i++) {
        dso_reporter_func(&works[i]);
    }

    // Clean up and free all resources
    free(threads);
    free(works);
    dlclose(handle);

    return 0;
}
