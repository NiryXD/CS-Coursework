#include <stdlib.h>
#include <string.h>
#include "plugin.h"

// No init or setting functions for flip plugin

PluginResult transform(char *data, int *size)
{
    if (*size == 0) {
        return PR_SUCCESS;
    }
    
    // First pass: count the lines
    int line_count = 0;
    int i;
    for (i = 0; i < *size; i++) {
        if (data[i] == '\n' || i == *size - 1) {
            line_count++;
        }
    }
    
    // If there's only one line or no lines, nothing to flip
    if (line_count <= 1) {
        return PR_SUCCESS;
    }
    
    // Allocate arrays for exactly the number of lines we need
    int *line_starts = malloc(line_count * sizeof(int));
    int *line_lengths = malloc(line_count * sizeof(int));
    
    if (!line_starts || !line_lengths) {
        free(line_starts);
        free(line_lengths);
        return PR_FAILED;
    }
    
    // Second pass: find all line positions
    int current_line = 0;
    i = 0;
    
    while (i < *size && current_line < line_count) {
        // Mark the start of this line
        line_starts[current_line] = i;
        
        // Find the end of this line
        int line_end = i;
        while (line_end < *size && data[line_end] != '\n') {
            line_end++;
        }
        
        // Include the newline if present
        if (line_end < *size && data[line_end] == '\n') {
            line_end++;
        }
        
        line_lengths[current_line] = line_end - i;
        current_line++;
        
        i = line_end;
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
    for (i = current_line - 1; i >= 0; i--) {
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