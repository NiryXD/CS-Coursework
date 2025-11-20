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
                fprintf(stderr, "ERROR: %s\n", dlerror());
                munmap(data, size);
                return 1;
            }

            INIT initFunction = (INIT)dlsym(handle, "init");
            SETTING settingFunction = (SETTING)dlsym(handle, "setting");
            TRANSFORM transformFunction = (TRANSFORM)dlsym(handle, "transform");

            if (!transformFunction) {
                fprintf (stderr, "ERROR: unable to find 'transform' function in plugin %s \n", pName);
                dlclose(handle);
                munmap(data, size);
                return 1;
            }

            if (initFunction) {
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
                if (settingFunction) {
                    PluginResult result = settingFunction(name, value);
                    if (result == PR_FAILED) {
                        fprintf(stderr, "ERROR: invalid plugin argument for %s: %s\n", pName, name);
                        dlclose(handle);
                        munmap(data, size);
                        return 1;
                    }
                    else if (result == PR_STOP) {
                        dlclose(handle);
                        munmap(data, size);
                        return 1;
                    }
                }
                else {
                    fprintf(stderr, "ERROR: %s does not take settings.\n", pName);
                    dlclose(handle);
                    munmap(data, size);
                    return 1;
                }
                i++;
            }
            // Run main transform funcion on file data
            PluginResult result = transformFunction(data, &size);
            if (result == PR_FAILED) {
                fprintf(stderr, "ERROR: plugin '%s' failed.\n", pName);
            }
            else if (result == PR_STOP) {
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

    fwrite(data, 1 , size, stdout);
    printf("\n");

    munmap(data, size);
    return 0;
}
