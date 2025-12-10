#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "plugin.h"

static char upper_map[27] = "abcdefghijklmnopqrstuvwxyz";
static char lower_map[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void init(void)
{
    // Reset to default mappings
    strcpy(upper_map, "abcdefghijklmnopqrstuvwxyz");
    strcpy(lower_map, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

PluginResult setting(const char name[], const char value[])
{
    int len = strlen(value);
    
    if (strcmp(name, "upper") == 0) {
        // Check if value is too long
        if (len > 26) {
            return PR_FAILED;
        }
        
        // Reset to default first
        strcpy(upper_map, "abcdefghijklmnopqrstuvwxyz");
        
        // Copy the provided characters
        int i;
        for (i = 0; i < len && i < 26; i++) {
            upper_map[i] = value[i];
        }
        
        return PR_SUCCESS;
        
    } else if (strcmp(name, "lower") == 0) {
        // Check if value is too long
        if (len > 26) {
            return PR_FAILED;
        }
        
        // Reset to default first
        strcpy(lower_map, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        
        // Copy the provided characters
        int i;
        for (i = 0; i < len && i < 26; i++) {
            lower_map[i] = value[i];
        }
        
        return PR_SUCCESS;
    }
    
    // Unknown setting
    return PR_FAILED;
}

PluginResult transform(char *data, int *size)
{
    int i;
    for (i = 0; i < *size; i++) {
        if (isupper(data[i])) {
            // Get index (A=0, B=1, etc.)
            int index = data[i] - 'A';
            if (index >= 0 && index < 26) {
                data[i] = upper_map[index];
            }
        } else if (islower(data[i])) {
            // Get index (a=0, b=1, etc.)
            int index = data[i] - 'a';
            if (index >= 0 && index < 26) {
                data[i] = lower_map[index];
            }
        }
        // Non-letter characters remain unchanged
    }
    
    return PR_SUCCESS;
}
