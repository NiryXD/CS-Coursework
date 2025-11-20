#include "plugin.h"
// Write your reverse plugin here

PluginResult transform(char *data, int *size) {
    int left = 0;
    int right = *size - 1;
    char temporary;

    while (left < right) {
        temporary = data[left];
        data[left] = data[right];
        data[right] = temporary;
        left++;
        right--;

    }

    return PR_SUCCESS;
}
