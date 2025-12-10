#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "plugin.h"

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s [-p plugin [-s setting=value]] ... file\n", argv[0]);
        return 1;
    }
    
    // Open the file (last argument)
    const char *filename = argv[argc - 1];
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    
    // Get file size
    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("fstat");
        close(fd);
        return 1;
    }
    
    // Map the file into memory
    int size = st.st_size;
    char *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }
    
    // Close file descriptor as it's no longer needed after mmap
    close(fd);
    
    // Process command line arguments
    int i = 1;
    while (i < argc - 1) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc - 1) {
            i++;
            
            // Build the plugin filename
            char plugin_name[256];
            snprintf(plugin_name, sizeof(plugin_name), "./lib%s.so", argv[i]);
            
            // Open the plugin
            void *handle = dlopen(plugin_name, RTLD_NOW);
            if (!handle) {
                fprintf(stderr, "ERROR: %s\n", dlerror());
                munmap(data, size);
                return 1;
            }
            
            // Look up the functions
            INIT init_func = (INIT)dlsym(handle, "init");
            SETTING setting_func = (SETTING)dlsym(handle, "setting");
            TRANSFORM transform_func = (TRANSFORM)dlsym(handle, "transform");
            
            // transform is required
            if (!transform_func) {
                fprintf(stderr, "ERROR: unable to find 'transform' function in plugin %s.\n", plugin_name);
                dlclose(handle);
                munmap(data, size);
                return 1;
            }
            
            // Call init if it exists
            if (init_func) {
                init_func();
            }
            
            // Move to next argument
            i++;
            
            // Process any settings for this plugin
            while (i < argc - 1 && strcmp(argv[i], "-s") == 0) {
                i++;
                if (i >= argc - 1) {
                    break;
                }
                
                // Parse the setting
                char *setting_str = argv[i];
                char name[256] = "";
                char value[256] = "";
                
                // Find the equals sign
                char *equals = strchr(setting_str, '=');
                if (equals) {
                    // Copy the name part
                    int name_len = equals - setting_str;
                    strncpy(name, setting_str, name_len);
                    name[name_len] = '\0';
                    
                    // Copy the value part
                    strcpy(value, equals + 1);
                } else {
                    // Just a name, no value
                    strcpy(name, setting_str);
                    value[0] = '\0';  // empty string
                }
                
                // Call the setting function if it exists
                if (setting_func) {
                    PluginResult result = setting_func(name, value);
                    if (result == PR_FAILED) {
                        fprintf(stderr, "ERROR: invalid plugin argument for %s: %s\n", plugin_name, name);
                        dlclose(handle);
                        munmap(data, size);
                        return 1;
                    } else if (result == PR_STOP) {
                        // Plugin handled the error, exit silently
                        dlclose(handle);
                        munmap(data, size);
                        return 1;
                    }
                } else {
                    // No setting function but setting was specified
                    fprintf(stderr, "ERROR: %s does not take settings.\n", plugin_name);
                    dlclose(handle);
                    munmap(data, size);
                    return 1;
                }
                
                i++;
            }
            
            // Call transform
            PluginResult result = transform_func(data, &size);
            if (result == PR_FAILED) {
                fprintf(stderr, "ERROR: plugin '%s' failed.\n", plugin_name);
                // Continue to next plugin
            } else if (result == PR_STOP) {
                // Fatal error, stop execution
                dlclose(handle);
                munmap(data, size);
                return 1;
            }
            
            // Close the plugin immediately after use
            dlclose(handle);
        } else {
            i++;
        }
    }
    
    // Print the final data
    fwrite(data, 1, size, stdout);
    printf("\n");
    
    // Clean up
    munmap(data, size);
    
    return 0;
}