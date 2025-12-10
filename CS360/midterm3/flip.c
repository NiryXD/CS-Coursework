#include "plugin.h"
// Write your flip plugin here

#include <string.h>
#include <stdlib.h>

PluginResult transform(char *data, int *size) {
    // If there's no data, nothing to do — report success.
    if (*size == 0) {
        return PR_SUCCESS;

    }

    // Count how many lines are in the input.
    // We treat a newline '\n' as the end of a line. If the file
    // doesn't end with a newline, the last character still counts
    // as the end of a line (that's why we check i == *size - 1).
    int lines = 0;
    for (int i = 0; i < *size; i++) {
        if (data[i] == '\n' || i == *size - 1) {
            lines++;
        }
    }

    // If there's zero or one line, flipping does nothing.
    if (lines <= 1) {
        return PR_SUCCESS;
    }

    // We'll record where each line starts and how long it is.
    // `lineBegin[i]` will be the index in `data` where line i starts.
    // `lineLength[i]` will be the number of bytes in that line,
    // including the trailing '\n' if it exists.
    int *lineBegin = malloc(lines * sizeof(int));
    int *lineLength = malloc(lines * sizeof(int));

    // If we can't get memory, clean up and report failure.
    if (!lineBegin || !lineLength) {
        free(lineBegin);
        free(lineLength);
        return PR_FAILED;
    }

    // Walk through `data` and fill the arrays with each line's
    // starting position and length. We include the newline
    // character in the length so the line stays intact when copied.
    int pLine = 0;
    int i = 0;

    while (i < *size && pLine < lines) {
        // Mark where this line starts
        lineBegin[pLine] = i;

        // Find the end of the line (stop at '\n' or at the end of data)
        int lineEnd = i;
        while (lineEnd < *size && data[lineEnd] != '\n') {
            lineEnd++;
        }

        // If we stopped at a '\n', include that character in the line's length
        if (lineEnd < *size && data[lineEnd] == '\n') {
            lineEnd++;
        }

        // Store the line length (end - start)
        lineLength[pLine] = lineEnd - i;
        pLine++;
        // Move to the next line start
        i = lineEnd;
    }

    // Create a temporary buffer to build the flipped output.
    // We keep the same overall size because we're just reordering lines,
    // not changing the content of the lines themselves.
    char *temporary = malloc(*size);
    if (!temporary) {
        free(lineBegin);
        free(lineLength);
        return PR_FAILED;
    }

    // Copy each line into the temporary buffer in reverse order.
    // `write` tracks where in `temporary` we are writing next.
    int write = 0;
    for (i = pLine - 1; i >= 0; i--) {
        // Copy the whole line (including '\n' if present)
        memcpy(temporary + write, data + lineBegin[i], lineLength[i]);
        write += lineLength[i];
    }

    // Put the flipped data back into the original buffer.
    // This overwrites `data` with the reversed-line version.
    memcpy(data, temporary, *size);

    // Clean up and report success.
    free(temporary);
    free(lineBegin);
    free(lineLength);
    return PR_SUCCESS;
}
