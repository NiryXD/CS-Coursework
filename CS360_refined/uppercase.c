#include "plugin.h"
// Write your uppercase plugin here

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static int stride = 1;

void init(void) {
    stride = 1;
}

PluginResult setting( const char name[], const char value[]){
    if (strcmp(name, "stride") == 0) {
        char *end;
        int nStride = strtol(value, &end, 10);

        if (*end != '\0' || nStride <= 0) {
            return PR_FAILED;
        }

        stride = nStride;
        return PR_SUCCESS;
    }

    return PR_FAILED;
}

PluginResult transform (char *data, int *size) {
    int count = 0;

    for (int i = 0; i < *size; i++) {
        if (isalpha(data[i])) {
            if (count % stride == 0) {
                data[i] = toupper(data[i]);

            }
            count++;
        }
    }
    return PR_SUCCESS;
}
