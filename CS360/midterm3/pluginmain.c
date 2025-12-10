#include <stdio.h>

#include "plugin.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>


int main(int argc, char *argv[])
{
    /*
     * High-level overview (plain English):
     * - This program reads a file into memory, loads one or more plugins
     *   (shared libraries) specified on the command line, and runs a
     *   plugin-provided `transform` function on the mapped file data.
     * - Command line options:
     *     - `-p <plugin>` : load the plugin named `<plugin>` (the code
     *       builds a library filename like `./lib<plugin>.so`). Multiple
     *       `-p` entries are allowed.
     *     - `-s name=value`: supply a setting to the most recently loaded
     *       plugin; the program will call the plugin's `setting` function
     *       with the parsed name and value.
     * - Plugin lifecycle:
     *     1) `dlopen` the plugin shared object.
     *     2) `dlsym` the plugin's `init`, `setting`, and `transform` symbols.
     *     3) Call `init()` if provided, call `setting()` for each `-s` arg,
     *        then call `transform(data, &size)` to let the plugin modify
     *        the mapped file contents.
     *     4) `dlclose` the plugin and continue.
     *
     * Important exam notes:
     * - `dlopen` returns a handle; check it for NULL and use `dlerror()`
     *   to get a readable error message.
     * - `dlsym` returns `void *`; cast it to the expected function-pointer
     *   type before use. Calling a symbol with the wrong signature is
     *   undefined behavior.
     * - `mmap` maps the file into memory for in-place modification. Using
     *   `MAP_PRIVATE` with `PROT_READ|PROT_WRITE` allows writing to the
     *   mapping without changing the original file (private copy-on-write).
     */

    const char *fn = argv[argc - 1];
    int fd = open(fn, O_RDONLY);
    if (fd < 0) {
        return 1;
    }

    // Gain insight into a file
    struct stat info;
    if (fstat(fd, &info) < 0 ) {
        close(fd);
        return 1;

    }


    // Map the file into mem
    int size = info.st_size;
    /*
     * We map the file into memory so plugins can inspect and modify the
     * bytes directly. `PROT_READ|PROT_WRITE` gives us a writable view,
     * and `MAP_PRIVATE` creates a private copy-on-write mapping so the
     * original file on disk is not overwritten by plugin changes.
     */
    char *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        close(fd);
        return 1;

    }

    close(fd);

    int i = 1;
    while (i < argc - 1){
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc - 1){
            i++;

            char pName[256];
            snprintf(pName, sizeof(pName), "./lib%s.so", argv[i]);


            //Build file name
            void *handle = dlopen(pName, RTLD_NOW);
            if (!handle) {
                /*
                 * If `dlopen` fails, `dlerror()` explains why (missing file,
                 * symbol problems, incompatible ABI, etc.). We must clean up
                 * the mapped memory and exit.
                 */
                fprintf(stderr, "ERROR: %s\n", dlerror());
                munmap(data, size);
                return 1;
            }

            INIT initFunction = (INIT)dlsym(handle, "init");
            SETTING settingFunction = (SETTING)dlsym(handle, "setting");
            TRANSFORM transformFunction = (TRANSFORM)dlsym(handle, "transform");

            if (!transformFunction) {
                /*
                 * The `transform` function is mandatory: plugins must provide
                 * it because it's the main entry point used to change the
                 * file data. If it's missing we close the plugin and exit.
                 */
                fprintf (stderr, "ERROR: unable to find 'transform' function in plugin %s \n", pName);
                dlclose(handle);
                munmap(data, size);
                return 1;
            }

            if (initFunction) {
                /* Optional init: let the plugin perform any one-time setup. */
                initFunction();
            }

            i++;

            while (i < argc - 1 && strcmp(argv[i], "-s") == 0) {
                i++;
                if (i >= argc - 1) {
                    break;
                }

                char *sString = argv[i];
                char name[256] = "";
                char value[256] = "";

                char *equal = strchr(sString, '=');
                if (equal) {
                    int nLength = equal - sString;
                    strncpy (name, sString, nLength);
                    name[nLength] = '\0';

                    strcpy(value, equal + 1);
                }
                else {
                    strcpy (name, sString);
                    value[0] = '\0';
                }

                // I had ChatGPT breakdown the fucntions like dlclose and dlsym
                /*
                 * Call the plugin's `setting` function (if present) to pass
                 * a name=value pair. The plugin can accept, reject, or
                 * request stopping via the returned `PluginResult`.
                 */
                if (settingFunction) {
                    PluginResult result = settingFunction(name, value);
                    if (result == PR_FAILED) {
                        /* Plugin rejected the setting: clean up and exit. */
                        fprintf(stderr, "ERROR: invalid plugin argument for %s: %s\n", pName, name);
                        dlclose(handle);
                        munmap(data, size);
                        return 1;
                    }
                    else if (result == PR_STOP) {
                        /* Plugin requested an immediate stop. */
                        dlclose(handle);
                        munmap(data, size);
                        return 1;
                    }
                }
                else {
                    /* Plugin doesn't accept settings but `-s` was provided. */
                    fprintf(stderr, "ERROR: %s does not take settings.\n", pName);
                    dlclose(handle);
                    munmap(data, size);
                    return 1;
                }
                i++;
            }
            // Run main transform funcion on file data
            /*
             * `transform` is expected to operate on the `data` buffer and may
             * resize it by updating `size`. It returns a `PluginResult` to
             * indicate success, failure, or a request to stop processing.
             */
            PluginResult result = transformFunction(data, &size);
            if (result == PR_FAILED) {
                fprintf(stderr, "ERROR: plugin '%s' failed.\n", pName);
            }
            else if (result == PR_STOP) {
                /* Plugin requested termination: clean up and exit. */
                dlclose(handle);
                munmap(data, size);
                return 1;
            }

            dlclose(handle);
        }
        else {
            i++;
        }
    }

    /*
     * Write the potentially-modified data to stdout so the caller can
     * redirect it to a file. We print a trailing newline for convenience.
     */
    fwrite(data, 1 , size, stdout);
    printf("\n");

    /* Clean up the memory mapping and exit. */
    munmap(data, size);
    return 0;
}
