#include "plugin.h"
// Write your reverse plugin here
/*
 * Reverse plugin (plain English):
 * - Purpose: reverse the bytes in the provided buffer `data` in place.
 * - How it works: use two indexes (pointers) that start at the left and
 *   right ends of the buffer and swap pairs of characters until they meet
 *   in the middle. This is a common, simple in-place reversal algorithm.
 * - Characteristics:
 *     * In-place: the function modifies the buffer directly; it does not
 *       allocate a new buffer.
 *     * Time complexity: O(n) where n is `*size` — each byte is visited
 *       at most once.
 *     * Space complexity: O(1) extra memory (only one temporary variable).
 * - Edge cases:
 *     * If `*size` is 0 or 1 the loop does nothing (already reversed).
 *     * This function treats the buffer as raw bytes; if the buffer holds
 *       multi-byte characters (e.g., UTF-8) reversing bytes may break
 *       character boundaries. For plain ASCII/byte-oriented data this is
 *       fine.
 */

PluginResult transform(char *data, int *size) {
    int left = 0;
    int right = *size - 1;
    char temporary;

    /* Swap characters from the ends moving toward the center. */
    while (left < right) {
        temporary = data[left];
        data[left] = data[right];
        data[right] = temporary;
        left++;
        right--;
    }

    /* The plugin contract expects a PluginResult; return success. */
    return PR_SUCCESS;
}