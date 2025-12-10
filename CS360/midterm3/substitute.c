#include "plugin.h"
// Write your substitute plugin here

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*
 * Substitute plugin (layman's explanation):
 *
 * Purpose:
 * - Replace letters in the input buffer with other letters according to
 *   two substitution alphabets: one used for uppercase letters and one
 *   for lowercase letters. This is similar to a simple substitution
 *   cipher where you map A->X, B->Y, etc.
 *
 * Key variables:
 * - `upper`: an array that gives the replacement character for 'A'..'Z'.
 * - `lower`: an array that gives the replacement character for 'a'..'z'.
 *
 * Plugin API (what the host calls):
 * - `init()`    : reset to defaults (full A..Z / a..z mapping).
 * - `setting()` : accept `upper` or `lower` settings to customize the
 *                 substitution alphabets (pass a partial or full string).
 * - `transform()` : apply the substitution to the provided `data` buffer
 *                   in-place.
 *
 * Behavior notes (exam-friendly):
 * - `setting("upper", "QWERTY...")` replaces the start of the
 *   uppercase mapping. If the provided value is shorter than 26 letters,
 *   only the first N letters are changed and the rest keep their default
 *   A..Z mapping.
 * - The code validates that settings are not longer than 26 characters
 *   (otherwise it rejects the setting with `PR_FAILED`).
 * - `transform` loops over every byte in the buffer; if the byte is an
 *   uppercase letter it uses `upper[idx]` (where idx='A'..'Z' -> 0..25)
 *   as the replacement, similarly for lowercase letters.
 * - Non-alphabetic characters are left unchanged.
 * - This works for plain ASCII text. If the buffer contains multi-byte
 *   UTF-8 characters the plugin will operate on raw bytes and may corrupt
 *   multi-byte characters; the host expects byte-oriented data here.
 */

static char upper[27] = "abcdefghijklmnopqrstuvwxyz";
static char lower[27] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

/* init: reset substitution mappings to the default identity mapping */
void init(void)  {
    strcpy(upper, "abcdefghijklmnopqrstuvwxyz");
    strcpy(lower, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

/*
 * setting: called for each `-s name=value` option. Supported names are
 * `upper` and `lower`. The `value` string provides replacement letters
 * starting at 'A' or 'a' respectively. Shorter strings update just the
 * prefix of the mapping; longer-than-26 strings are rejected.
 */
PluginResult setting(const char name[], const char value[]) {
    int length = strlen(value);
    
    if (strcmp(name, "upper" ) == 0) {
        if (length > 26) {
            return PR_FAILED; /* too long */
        }

        /* Start with default mapping then overwrite the prefix */
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

    /* Unknown setting name */
    return PR_FAILED;
}

/*
 * transform: apply substitutions in-place. For each character:
 * - If it's uppercase: compute index = c - 'A' and replace with upper[index]
 * - If it's lowercase: compute index = c - 'a' and replace with lower[index]
 * - Otherwise leave the character unchanged.
 */
PluginResult transform (char *data, int *size) {
    for (int i = 0; i < *size; i++) {
        if (isupper((unsigned char)data[i])) {
            int idx = data[i] - 'A';
            if (idx >= 0 && idx < 26) {
                data[i] = upper[idx];
            }
        }

        else if (islower((unsigned char)data[i])) {
            int idx = data[i] - 'a';
            if (idx >= 0 && idx < 26) {
                data[i] = lower[idx];
            }
        }
    }

    return PR_SUCCESS;
}