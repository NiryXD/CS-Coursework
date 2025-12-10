#include <stdlib.h>
#include <string.h>
#include "plugin.h"

// No init or setting functions for flip plugin

PluginResult transform(char *data, int *size)
{
    if (*size == 0) {
        return PR_SUCCESS;
    }
    
    // Allocate arrays to store line start and end positions
    int max_lines = 100;
    int *line_starts = malloc(max_lines * sizeof(int));
    int *line_lengths = malloc(max_lines * sizeof(int));
    
    if (!line_starts || !line_lengths) {
        free(line_starts);
        free(line_lengths);
        return PR_FAILED;
    }
    
    // Find all lines
    int line_count = 0;
    int i = 0;
    
    while (i < *size) {
        // Reallocate if we need more space
        if (line_count >= max_lines) {
            max_lines *= 2;
            line_starts = realloc(line_starts, max_lines * sizeof(int));
            line_lengths = realloc(line_lengths, max_lines * sizeof(int));
            if (!line_starts || !line_lengths) {
                free(line_starts);
                free(line_lengths);
                return PR_FAILED;
            }
        }
        
        // Mark the start of this line
        line_starts[line_count] = i;
        
        // Find the end of this line
        int line_end = i;
        while (line_end < *size && data[line_end] != '\n') {
            line_end++;
        }
        
        // Include the newline if present
        if (line_end < *size && data[line_end] == '\n') {
            line_end++;
        }
        
        line_lengths[line_count] = line_end - i;
        line_count++;
        
        i = line_end;
    }
    
    // If there's only one line or no lines, nothing to flip
    if (line_count <= 1) {
        free(line_starts);
        free(line_lengths);
        return PR_SUCCESS;
    }
    
    // Create a temporary buffer
    char *temp = malloc(*size);
    if (!temp) {
        free(line_starts);
        free(line_lengths);
        return PR_FAILED;
    }
    
    // Copy lines in reverse order
    int write_pos = 0;
    for (i = line_count - 1; i >= 0; i--) {
        memcpy(temp + write_pos, data + line_starts[i], line_lengths[i]);
        write_pos += line_lengths[i];
    }
    
    // Copy back to original buffer
    memcpy(data, temp, *size);
    
    // Clean up
    free(temp);
    free(line_starts);
    free(line_lengths);
    
    return PR_SUCCESS;
}
