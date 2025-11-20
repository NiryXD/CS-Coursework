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

   if (strcmp(exports->key, argv[2]) != 0) {
    printf("ERROR: key does not match.\n");
    dlclose(handle);
    return 1;
   }
   char buffer[256];
   exports->input(buffer);

   printf("Input = '%s'\n", buffer);

   int len = strlen(buffer);
   for (int i = 0; i < len / 2; i++) {
    char temp = buffer[i];
    buffer[i] = buffer[len - 1 - i];
    buffer[len - 1 - i] = temp;
   }

    exports->output(buffer);
    dlclose(handle);
    return 0;
}
