#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "plugin.h"

static int stride = 1;

void init(void)
{
    // Reset stride to default value
    stride = 1;
}

PluginResult setting(const char name[], const char value[])
{
    if (strcmp(name, "stride") == 0) {
        // Parse the stride value
        char *endptr;
        int new_stride = strtol(value, &endptr, 10);
        
        // Check if conversion was successful and value is valid
        if (*endptr != '\0' || new_stride <= 0) {
            return PR_FAILED;
        }
        
        stride = new_stride;
        return PR_SUCCESS;
    }
    
    // Unknown setting
    return PR_FAILED;
}

PluginResult transform(char *data, int *size)
{
    int i;
    int letter_count = 0;
    
    for (i = 0; i < *size; i++) {
        if (isalpha(data[i])) {
            // Check if this is the nth letter based on stride
            if (letter_count % stride == 0) {
                data[i] = toupper(data[i]);
            }
            letter_count++;
        }
    }
    
    return PR_SUCCESS;
}
