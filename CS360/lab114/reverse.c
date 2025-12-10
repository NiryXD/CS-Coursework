#include "plugin.h"

// No init or setting functions for reverse plugin

PluginResult transform(char *data, int *size)
{
    int left = 0;
    int right = *size - 1;
    char temp;
    
    // Swap characters from both ends moving toward the middle
    while (left < right) {
        temp = data[left];
        data[left] = data[right];
        data[right] = temp;
        left++;
        right--;
    }
    
    return PR_SUCCESS;
}
