#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>

typedef void (*INPUTFUNC)(char dest[]);
typedef void (*OUTPUTFUNC)(const char src[]);

struct Export {
    INPUTFUNC input;
    OUTPUTFUNC output;
    char key[16];
};

/*
 * Plain-English explanation of Dynamic Shared Objects (DSOs) and the
 * runtime linker functions used here:
 *
 * - A DSO (shared library, commonly a .so file on Unix) is a separately
 *   compiled chunk of code that can be loaded into a running program.
 *   This lets programs use plugins or replaceable modules without being
 *   recompiled.
 *
 * - `dlopen(libname, flags)` opens a shared library at runtime and
 *   returns a handle used to access symbols (functions/variables) from it.
 *   The `RTLD_LAZY` flag means "look up function addresses when they're
 *   actually used" rather than immediately.
 *
 * - `dlsym(handle, "symbol")` returns a pointer to the named symbol
 *   inside the loaded library. You must cast that `void *` to the correct
 *   function or data pointer type before using it.
 *
 * - `dlclose(handle)` closes the library when you're done.
 *
 * - Common pitfalls to remember for an exam:
 *     * Always check for NULL from `dlopen`/`dlsym` and use `dlerror()` to
 *       get a human-readable message explaining failures.
 *     * `dlsym` returns `void *` — casting to the wrong function pointer
 *       type leads to undefined behavior.
 *     * Symbols and ABIs must match between the main program and the
 *       library (same function signatures, calling convention, etc.).
 */

int main(int argc, char *argv[])
{
    if (3 != argc) {
        fprintf(stderr, "Usage: %s <library> <key>\n", argv[0]);
        return 1;
    }
    char libname[256];
    const char *stem = argv[1];
    int lib_yes = (strncmp(stem, "lib", 3) == 0);

    int length = strlen(stem);
    int so_yes = (length >= 3 && strcmp(stem + length - 3, ".so") == 0);

    if (lib_yes && so_yes) {
        snprintf(libname, sizeof(libname), "./%s", stem);
    } else if (lib_yes && !so_yes) {
        snprintf(libname, sizeof(libname), "./%s.so", stem);
    } else if (!lib_yes && so_yes) {
        snprintf(libname, sizeof(libname), "./lib%s", stem);
    } else {
        snprintf(libname, sizeof(libname), "./lib%s.so", stem);
    }

    /*
     * The previous block builds a filename so users can pass either
     * "mylib", "libmylib", "mylib.so" or "libmylib.so" and the program
     * will try to open the sensible local library path (./libmylib.so).
     */

    void *handle = dlopen(libname, RTLD_LAZY);
    if (!handle) {
        printf("%s\n", dlerror());
        return 1;
    }

    struct Export *exports = (struct Export *)dlsym(handle, "EXPORTS");
    if (!exports) {
        printf("%s\n", dlerror());
        dlclose(handle);
        return 1;
    }

    /*
     * `EXPORTS` is expected to be a symbol exported by the shared library
     * with the type `struct Export`. That means the library should define
     * and export a variable like:
     *   struct Export EXPORTS = { .input = my_input, .output = my_output, .key = "..." };
     *
     * Here we treat the pointer returned from `dlsym` as a pointer to that
     * struct so we can call the input/output function pointers inside it.
     */

   if (strcmp(exports->key, argv[2]) != 0) {
    printf("ERROR: key does not match.\n");
    dlclose(handle);
    return 1;
   }
   char buffer[256];
    /*
     * Call the library-provided `input` function. We pass a local buffer so
     * the library can write some text into it. Because `input` has the
     * signature `void (*)(char[])` we pass `buffer` directly.
     */
    exports->input(buffer);

   printf("Input = '%s'\n", buffer);

   int len = strlen(buffer);
   for (int i = 0; i < len / 2; i++) {
    char temp = buffer[i];
    buffer[i] = buffer[len - 1 - i];
    buffer[len - 1 - i] = temp;
   }

    /*
     * After reversing the string we call the library's `output` function
     * to hand the modified buffer back to the plugin. Again we call via
     * the function pointer stored in the `EXPORTS` struct.
     *
     * Finally, close the library handle with `dlclose` to release the
     * runtime linker's resources. On many systems the code remains mapped
     * until process exit but `dlclose` is the correct API to use.
     */
    exports->output(buffer);
    dlclose(handle);
    return 0;
}