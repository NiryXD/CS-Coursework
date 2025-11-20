#include "plugin.h"
// Write your flip plugin here

#include <string.h>
#include <stdlib.h>

PluginResult transform(char *data, int *size) {
    if (*size == 0) {
        return PR_SUCCESS;

    }

    int lines = 0;
    for (int i = 0; i < *size; i++) {
        if (data[i] == '\n' || i == *size - 1) {
            lines++;
        }
    }

    if (lines <= 1) {
        return PR_SUCCESS;
    }

    int *lineBegin = malloc(lines * sizeof(int));
    int *lineLength = malloc(lines * sizeof(int));

    if (!lineBegin || !lineLength) {
        free(lineBegin);
        free(lineLength);
        return PR_FAILED;
    }

    int pLine = 0;
    int i = 0;

    while (i < *size && pLine < lines) {
        lineBegin[pLine] = i;

        int lineEnd = i;
        while (lineEnd < *size && data[lineEnd] != '\n') {
            lineEnd++;
        }

        if (lineEnd < *size && data[lineEnd] == '\n') {
            lineEnd++;
        }

        lineLength[pLine] = lineEnd - i;
        pLine++;
        i = lineEnd;
    }

    char *temporary = malloc(*size);
    if (!temporary) {
        free(lineBegin);
        free(lineLength);
        return PR_FAILED;
    }

    int write = 0;
    for (i = pLine - 1; i >= 0; i--) {
        memcpy(temporary + write, data + lineBegin[i], lineLength[i]);
        write += lineLength[i];
    }

    memcpy(data, temporary, *size);
    free(temporary);
    free(lineBegin);
    free(lineLength);
    return PR_SUCCESS;
}
