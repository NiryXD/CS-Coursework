#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#include "plugin.h"
#include "tpool.h"

// Plugin shared object handle stored in a static for simplicity.
// Thread pool is not global, but the plugin handle can be.
static void *plugin_handle = NULL;

// Helper to try multiple library name variants and load plugin.
// On success:
//   - sets plugin_handle
//   - sets *p_init, *p_setting, *p_transform
//   - returns 0
// On failure: prints error and returns nonzero.
static int load_plugin(const char *stem,
                       INIT *p_init,
                       SETTING *p_setting,
                       TRANSFORM *p_transform)
{
    const int MAX_PATH = 512;
    char path[MAX_PATH];

    const char *candidates[8];
    int n = 0;

    // Candidate 0: raw stem as given
    candidates[n++] = stem;

    // The rest we construct with strdup so we can free them later
    snprintf(path, sizeof(path), "./%s", stem);
    candidates[n++] = strdup(path);

    snprintf(path, sizeof(path), "%s.so", stem);
    candidates[n++] = strdup(path);

    snprintf(path, sizeof(path), "./%s.so", stem);
    candidates[n++] = strdup(path);

    snprintf(path, sizeof(path), "lib%s", stem);
    candidates[n++] = strdup(path);

    snprintf(path, sizeof(path), "./lib%s", stem);
    candidates[n++] = strdup(path);

    snprintf(path, sizeof(path), "lib%s.so", stem);
    candidates[n++] = strdup(path);

    snprintf(path, sizeof(path), "./lib%s.so", stem);
    candidates[n++] = strdup(path);

    void *handle = NULL;
    const char *err = NULL;

    // Try each candidate name until one loads
    for (int i = 0; i < n; i++) {
        handle = dlopen(candidates[i], RTLD_LAZY);
        if (handle) {
            break;
        }
        err = dlerror();
    }

    // Free any strdup'ed strings (we only strdup indices 1..7)
    for (int i = 1; i < n; i++) {
        free((void *)candidates[i]);
    }

    if (!handle) {
        fprintf(stderr, "Failed to load plugin '%s': %s\n",
                stem, err ? err : "unknown error");
        return 1;
    }

    plugin_handle = handle;

    // Resolve symbols. init and setting are optional, transform is required.

    dlerror(); // clear any old error
    INIT init_func = (INIT)dlsym(handle, "init");
    const char *e1 = dlerror();
    if (e1) {
        // init is optional; if not found, just use NULL
        init_func = NULL;
    }

    dlerror();
    SETTING setting_func = (SETTING)dlsym(handle, "setting");
    const char *e2 = dlerror();
    if (e2) {
        // setting is optional
        setting_func = NULL;
    }

    dlerror();
    TRANSFORM transform_func = (TRANSFORM)dlsym(handle, "transform");
    const char *e3 = dlerror();
    if (e3 || !transform_func) {
        fprintf(stderr,
                "ERROR: required symbol 'transform' not found in plugin '%s': %s\n",
                stem, e3 ? e3 : "unknown error");
        dlclose(handle);
        plugin_handle = NULL;
        return 1;
    }

    *p_init = init_func;
    *p_setting = setting_func;
    *p_transform = transform_func;

    return 0;
}

// Helper to split "name=value" into separate name and value buffers.
// If there is no '=', the whole string is the name and value is "".
static void split_setting(const char *arg,
                          char *name, size_t name_sz,
                          char *value, size_t value_sz)
{
    const char *eq = strchr(arg, '=');
    if (!eq) {
        // No equal sign: entire arg is name, value is empty string
        snprintf(name, name_sz, "%s", arg);
        if (value_sz > 0) {
            value[0] = '\0';
        }
        return;
    }

    size_t nlen = (size_t)(eq - arg);
    if (nlen >= name_sz) {
        nlen = name_sz - 1;
    }
    memcpy(name, arg, nlen);
    name[nlen] = '\0';

    snprintf(value, value_sz, "%s", eq + 1);
}

int main(int argc, char *argv[])
{
    if (argc < 4) {
        fprintf(stderr,
                "Usage: %s <num_threads> <plugin> [-s name=value ...] <file1> [file2 ...]\n",
                argv[0]);
        return 1;
    }

    // 1. Parse number of threads
    int num_threads = 0;
    if (sscanf(argv[1], "%d", &num_threads) != 1 ||
        num_threads < 1 || num_threads > 32) {
        fprintf(stderr, "Invalid number of threads: '%s'\n", argv[1]);
        return 1;
    }

    const char *plugin_stem = argv[2];

    // 2. Load plugin and get function pointers
    INIT init_func = NULL;
    SETTING setting_func = NULL;
    TRANSFORM transform_func = NULL;

    if (load_plugin(plugin_stem, &init_func, &setting_func, &transform_func) != 0) {
        // Error already printed by load_plugin
        return 1;
    }

    // If plugin provides init, call it once before using settings or transform
    if (init_func) {
        init_func();
    }

    // 3. Parse -s settings and figure out where the file list starts.
    // Arguments look like:
    //   argv[0] = program
    //   argv[1] = num_threads
    //   argv[2] = plugin
    //   argv[3...] = [-s name=value ...] file1 file2 ...
    int i = 3;
    for (; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            i++;
            if (i >= argc) {
                fprintf(stderr, "ERROR: -s requires an argument name=value\n");
                dlclose(plugin_handle);
                return 1;
            }

            if (!setting_func) {
                fprintf(stderr,
                        "ERROR: plugin does not accept settings, but -s was used\n");
                dlclose(plugin_handle);
                return 1;
            }

            char name[256];
            char value[256];
            split_setting(argv[i], name, sizeof(name), value, sizeof(value));

            PluginResult pr = setting_func(name, value);
            if (pr == PR_FAILED) {
                fprintf(stderr, "ERROR: invalid plugin argument '%s'\n", name);
                dlclose(plugin_handle);
                return 1;
            } else if (pr == PR_STOP) {
                // Plugin requested immediate stop
                dlclose(plugin_handle);
                return 0;
            }
        } else {
            // First non -s token is the first file name
            break;
        }
    }

    if (i >= argc) {
        fprintf(stderr, "ERROR: no input files specified\n");
        dlclose(plugin_handle);
        return 1;
    }

    int file_start = i;
    int nfiles = argc - file_start;

    // Convenience pointer: array of file name strings
    char **filenames = &argv[file_start];

    // 4. Open thread pool
    void *pool = thread_pool_open(num_threads);
    if (!pool) {
        fprintf(stderr, "ERROR: could not initialize thread pool\n");
        dlclose(plugin_handle);
        return 1;
    }

    // 5. Compute original hashes in parallel with hash32

    // Allocate array of file descriptors
    int *fds = (int *)calloc(nfiles, sizeof(int));
    if (!fds) {
        fprintf(stderr, "ERROR: out of memory\n");
        thread_pool_close(pool);
        dlclose(plugin_handle);
        return 1;
    }

    // Open all files read-only to compute original hash
    for (int k = 0; k < nfiles; k++) {
        fds[k] = open(filenames[k], O_RDONLY);
        if (fds[k] < 0) {
            perror(filenames[k]);

            // For simplicity in an exam, treat failure as fatal:
            for (int j = 0; j < k; j++) {
                close(fds[j]);
            }
            free(fds);
            thread_pool_close(pool);
            dlclose(plugin_handle);
            return 1;
        }
    }

    // Submit all file descriptors as work items to the thread pool
    uint64_t *orig_hashes = thread_pool_execute(pool, fds, nfiles, hash32);

    // After hashing, we can close the file descriptors
    for (int k = 0; k < nfiles; k++) {
        close(fds[k]);
    }

    if (!orig_hashes) {
        fprintf(stderr, "ERROR: hashing original files failed\n");
        free(fds);
        thread_pool_close(pool);
        dlclose(plugin_handle);
        return 1;
    }

    // 6. Transform files using mmap and the plugin's transform function
    for (int k = 0; k < nfiles; k++) {
        int fd = open(filenames[k], O_RDWR);
        if (fd < 0) {
            perror(filenames[k]);
            // Skip this file but continue with others
            continue;
        }

        struct stat st;
        if (fstat(fd, &st) < 0) {
            perror("fstat");
            close(fd);
            continue;
        }

        if (st.st_size == 0) {
            // Nothing to transform
            close(fd);
            continue;
        }

        int size = (int)st.st_size;
        char *data = mmap(NULL, st.st_size,
                          PROT_READ | PROT_WRITE,
                          MAP_SHARED, fd, 0);
        if (data == MAP_FAILED) {
            perror("mmap");
            close(fd);
            continue;
        }

        PluginResult pr = transform_func(data, &size);
        if (pr == PR_FAILED) {
            fprintf(stderr,
                    "ERROR: plugin transform failed on file '%s'\n",
                    filenames[k]);
            // We still unmap and close before moving on
        } else if (pr == PR_STOP) {
            // Plugin asked us to stop processing any further files

            munmap(data, st.st_size);
            close(fd);

            free(fds);
            free(orig_hashes);
            thread_pool_close(pool);
            dlclose(plugin_handle);
            return 0;
        }

        // Unmap and close file after transformation
        munmap(data, st.st_size);
        close(fd);
    }

    // 7. Compute new hashes after transformation (using hash64)

    for (int k = 0; k < nfiles; k++) {
        fds[k] = open(filenames[k], O_RDONLY);
        if (fds[k] < 0) {
            perror(filenames[k]);

            for (int j = 0; j < k; j++) {
                close(fds[j]);
            }
            free(fds);
            free(orig_hashes);
            thread_pool_close(pool);
            dlclose(plugin_handle);
            return 1;
        }
    }

    uint64_t *new_hashes = thread_pool_execute(pool, fds, nfiles, hash64);

    for (int k = 0; k < nfiles; k++) {
        close(fds[k]);
    }

    if (!new_hashes) {
        fprintf(stderr, "ERROR: hashing transformed files failed\n");
        free(fds);
        free(orig_hashes);
        thread_pool_close(pool);
        dlclose(plugin_handle);
        return 1;
    }

    // 8. Print a summary line per file, then clean up.
    // We print the original 32 bit hash and the new 64 bit hash.
    for (int k = 0; k < nfiles; k++) {
        printf("%s  0x%08lx  0x%016lx\n",
               filenames[k],
               (unsigned long)orig_hashes[k],   // original (32 bit inside 64)
               (unsigned long)new_hashes[k]);   // new (full 64 bit)
    }

    free(fds);
    free(orig_hashes);
    free(new_hashes);

    thread_pool_close(pool);

    if (plugin_handle) {
        dlclose(plugin_handle);
    }

    return 0;
}
c