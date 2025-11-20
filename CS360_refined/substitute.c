#include "plugin.h"
// Write your substitute plugin here

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static char upper[27] = "abcdefghijklmnopqrstuvwxyz";
static char lower[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

void init(void)  {
    strcpy(upper, "abcdefghijklmnopqrstuvwxyz");
    strcpy(lower, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

PluginResult setting(const char name[], const char value[]) {
    int length = strlen(value);
    
    if (strcmp(name, "upper" ) == 0) {
        if (length > 26) {
            return PR_FAILED;
        }

        strcpy(upper, "abcdefghijklmnopqrstuvwxyz");

        for ( int i = 0; i < length && i < 26; i++) {
            upper[i] = value[i];
        }

        return PR_SUCCESS;
    }

    else if (strcmp(name, "lower") == 0) {
        if (length > 26) {
            return PR_FAILED;
        }

        strcpy(lower, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

        for (int i = 0; i < length && i < 26; i++) {
            lower[i] = value[i];
        }

        return PR_SUCCESS;
    }

    return PR_FAILED;
}

PluginResult transform (char *data, int *size) {
    for (int i = 0; i < *size; i++) {
        if (isupper(data[i])) {
            int idx = data[i] - 'A';
            if (idx >= 0 && idx < 26) {
                data[i] = upper[idx];
            }
        }

        else if (islower(data[i])) {
            int idx = data[i] - 'a';
            if (idx >= 0 && idx < 26) {
                data[i] = lower[idx];
            }
        }
    }

    return PR_SUCCESS;
}
