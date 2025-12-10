#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include "plugin.h"

typedef struct {
    char name[256];
    void *handle;
    INIT init_func;
    SETTING setting_func;
    TRANSFORM transform_func;
} Plugin;

int main(int argc, char *argv[])
{
    Plugin plugins[100];
    int plugin_count = 0;
    char *filename = NULL;
    int i;
    
    // Parse command line arguments
    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: -p requires an argument\n");
                return 1;
            }
            
            // Store plugin name for loading later
            i++;
            snprintf(plugins[plugin_count].name, sizeof(plugins[plugin_count].name), 
                     "./lib%s.so", argv[i]);
            plugin_count++;
            
        } else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "ERROR: -s requires an argument\n");
                return 1;
            }
            
            // Settings will be processed after loading plugins
            i++;
            // Continue parsing
            
        } else if (i == argc - 1) {
            // Last argument should be the filename
            filename = argv[i];
        }
    }
    
    // Check if we have a filename
    if (filename == NULL) {
        fprintf(stderr, "ERROR: No filename provided\n");
        return 1;
    }
    
    // Open the file
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("ERROR: Cannot open file");
        return 1;
    }
    
    // Get file size
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("ERROR: Cannot get file stats");
        close(fd);
        return 1;
    }
    
    // Map the file into memory with read/write for program, but not file
    char *data = mmap(NULL, st.st_size, PROT_READ | PROT_WRITE, 
                      MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("ERROR: mmap failed");
        close(fd);
        return 1;
    }
    
    // Close the file descriptor (no longer needed after mmap)
    close(fd);
    
    int size = st.st_size;
    
    // Now process plugins in order
    int current_plugin = 0;
    for (i = 1; i < argc && current_plugin < plugin_count; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            i++; // Skip the plugin name
            
            // Load the current plugin
            Plugin *p = &plugins[current_plugin];
            
            // Open the plugin
            p->handle = dlopen(p->name, RTLD_NOW);
            if (!p->handle) {
                fprintf(stderr, "ERROR: Cannot open plugin %s: %s\n", 
                        p->name, dlerror());
                munmap(data, st.st_size);
                return 1;
            }
            
            // Load the functions
            p->init_func = (INIT)dlsym(p->handle, "init");
            p->setting_func = (SETTING)dlsym(p->handle, "setting");
            p->transform_func = (TRANSFORM)dlsym(p->handle, "transform");
            
            // Check if transform exists (required)
            if (!p->transform_func) {
                fprintf(stderr, "ERROR: unable to find 'transform' function in plugin %s.\n", 
                        p->name);
                dlclose(p->handle);
                munmap(data, st.st_size);
                return 1;
            }
            
            // Call init if it exists
            if (p->init_func) {
                p->init_func();
            }
            
            // Process settings for this plugin
            int j = i + 1;
            while (j < argc && strcmp(argv[j], "-p") != 0 && j < argc - 1) {
                if (strcmp(argv[j], "-s") == 0 && j + 1 < argc) {
                    j++;
                    char name[256] = {0};
                    char value[256] = {0};
                    
                    // Parse setting=value
                    char *eq = strchr(argv[j], '=');
                    if (eq) {
                        strncpy(name, argv[j], eq - argv[j]);
                        name[eq - argv[j]] = '\0';
                        strcpy(value, eq + 1);
                    } else {
                        strcpy(name, argv[j]);
                        value[0] = '\0'; // Empty string
                    }
                    
                    // Apply setting if setting function exists
                    if (p->setting_func) {
                        PluginResult result = p->setting_func(name, value);
                        if (result == PR_FAILED) {
                            fprintf(stderr, "ERROR: invalid plugin argument for %s: %s\n", 
                                    p->name, name);
                            dlclose(p->handle);
                            munmap(data, st.st_size);
                            return 1;
                        } else if (result == PR_STOP) {
                            // Plugin handled the error
                            dlclose(p->handle);
                            munmap(data, st.st_size);
                            return 1;
                        }
                    } else {
                        // No setting function but setting was specified
                        fprintf(stderr, "ERROR: %s does not take settings.\n", p->name);
                        dlclose(p->handle);
                        munmap(data, st.st_size);
                        return 1;
                    }
                }
                j++;
            }
            
            // Call transform
            PluginResult result = p->transform_func(data, &size);
            if (result == PR_FAILED) {
                fprintf(stderr, "ERROR: plugin '%s' failed.\n", p->name);
                // Continue to next plugin
            } else if (result == PR_STOP) {
                // Fatal error, stop
                dlclose(p->handle);
                munmap(data, st.st_size);
                return 1;
            }
            
            // Close the plugin
            dlclose(p->handle);
            current_plugin++;
        }
    }
    
    // Print the final data
    for (i = 0; i < size; i++) {
        putchar(data[i]);
    }
    putchar('\n');
    
    // Clean up
    munmap(data, st.st_size);
    
    return 0;
}
