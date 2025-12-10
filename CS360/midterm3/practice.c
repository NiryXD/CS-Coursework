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
    /*
     * Dynamic Shared Object (DSO) overview - plain English for the exam:
     *
     * - What is a DSO?  A DSO (shared library, usually a `.so` file on
     *   Unix) is a separately-compiled module you can load at runtime. This
     *   lets the main program accept plugins or replaceable components
     *   without recompiling the whole program.
     *
     * - Key runtime functions:
     *     * `dlopen(path, flags)`  : load the library file and return a handle.
     *     * `dlsym(handle, name)`  : look up a symbol (function or variable)
     *                             inside the loaded library and return a
     *                             pointer to it (as `void *`).
     *     * `dlclose(handle)`     : drop the handle when done (cleanup).
     *     * `dlerror()`            : when `dlopen`/`dlsym` fail, call this to
     *                             get a human-readable error string.
     *
     * - Usage pattern in this program:
     *     1) Try to open the library using several common filename variants
     *        (with/without `./`, with/without `lib` prefix, and with/without
     *        the `.so` suffix) so the caller can pass different forms.
     *     2) Use `dlsym` to fetch two function symbols expected from the
     *        plugin: `dso_worker_func` and `dso_reporter_func`.
     *     3) Cast the returned `void *` from `dlsym` to the correct
     *        function-pointer type before calling it.
     *
     * - Important exam tips / pitfalls:
     *     * Always check `dlopen`/`dlsym` return values and use `dlerror()`
     *       to diagnose problems.
     *     * `dlsym` returns `void *`. You must cast that to the exact
     *       function-pointer type used by the program. Mismatched types are
     *       undefined behavior (can crash or corrupt state).
     *     * The library's function signatures and any shared struct layouts
     *       (like `struct Work`) must match exactly between the program
     *       and the plugin — the ABI must be compatible.
     *     * Be careful with ownership/lifetime: do not return pointers to
     *       stack memory across the DSO boundary; prefer heap or caller-
     *       allocated buffers or clearly-documented ownership rules.
     *     * Thread-safety: if the DSO exports functions that are invoked from
     *       multiple threads (like here, worker threads), those functions
     *       must be safe to call concurrently or must perform their own
     *       synchronization.
     *
     * - In this program specifically:
     *     * We expect the DSO to provide `dso_worker_func` with type
     *       `void *(*)(void *)` so it can be used directly with
     *       `pthread_create` as a thread entry point. The DSO's worker
     *       will be passed a pointer to a `struct Work` allocated by the
     *       main program.
     *     * The DSO should also provide `dso_reporter_func` with type
     *       `void (*)(const struct Work *)` so the main program can call
     *       it after joining threads to report results.
     */
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