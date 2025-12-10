#include "plugin.h"

/*
 * Uppercase plugin (plain explanation):
 * - Purpose: scan the provided `data` buffer and convert some letters to
 *   upper-case. Which letters get uppercased is controlled by `stride`.
 *
 * - Plugin API used by the host program (see `plugin.h`):
 *     * `init(void)`
 *         - Called once when the plugin is loaded. Use it to set
 *           default values.
 *     * `setting(const char name[], const char value[])`
 *         - Called for each `-s name=value` option the user provides.
 *           Return `PR_SUCCESS` if you accepted the setting, or
 *           `PR_FAILED` if the setting is invalid.
 *     * `transform(char *data, int *size)`
 *         - The main work function. The host passes a writable buffer and
 *           its size. Modify the buffer in-place and update `*size` if
 *           you change the length. Return a `PluginResult` status.
 *
 * - Simple rules for this plugin:
 *     * `stride` says "uppercase every Nth alphabetic character". For
 *       example, `stride=1` uppercases every letter; `stride=2` uppercases
 *       every other letter; `stride=3` uppercases one letter, then skips
 *       two, and so on.
 *     * `setting("stride", "3")` parses the number and updates the
 *       behavior. Invalid values (non-numeric or <= 0) cause `PR_FAILED`.
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/* How many alphabetic characters to skip between uppercased letters. */
static int stride = 1;

/* init: reset plugin state to sensible defaults. The host may call this
 * before any settings are applied. */
void init(void) {
    stride = 1;
}

/* setting: accept name/value pairs from the command line. This plugin
 * supports a single setting: `stride`. The function must validate the
 * value and return `PR_SUCCESS` on success or `PR_FAILED` on bad input.
 */
PluginResult setting( const char name[], const char value[]) {
    if (strcmp(name, "stride") == 0) {
        char *end;
        int nStride = strtol(value, &end, 10);

        /* If the string wasn't a valid positive integer, reject it. */
        if (*end != '\0' || nStride <= 0) {
            return PR_FAILED;
        }

        stride = nStride;
        return PR_SUCCESS;
    }

    /* Unknown setting name. */
    return PR_FAILED;
}

/* transform: walk the buffer and uppercase letters according to `stride`.
 * Implementation notes (layman's terms):
 * - `count` tracks how many alphabetic characters we've seen so far.
 * - When `count % stride == 0` we uppercase that alphabetic character.
 * - Non-letter bytes don't affect the `count` but are left unchanged.
 */
PluginResult transform (char *data, int *size) {
    int count = 0;

    for (int i = 0; i < *size; i++) {
        if (isalpha((unsigned char)data[i])) {
            /* If this is the Nth alphabetic character (based on stride),
             * uppercase it. Using `count % stride == 0` means the first
             * alphabetic character is uppercased when count==0.
             */
            if (count % stride == 0) {
                data[i] = toupper((unsigned char)data[i]);
            }
            count++;
        }
    }
    return PR_SUCCESS;
}