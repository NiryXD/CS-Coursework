/*
 * Program Description
 * -------------------
 * What this file does:
 *   A tiny growable string type backed by heap memory. It keeps:
 *     - buffer: pointer to a char array that holds the characters
 *     - length: how many characters are currently in the string (not counting '\0')
 *     - capacity: how many bytes are allocated for buffer (must be >= length + 1)
 *
 * How it works in plain steps:
 *   - string_new            : create an empty string "" with capacity 1 so it can store the '\0'
 *   - string_new_from(cstr) : create a new string by copying a C string
 *   - string_realloc        : grow the buffer to a requested capacity (if larger)
 *   - string_free           : free both the buffer and the struct
 *   - string_push           : append one character to the end, keeping the string NUL-terminated
 *   - string_append         : append a whole C string by pushing each character
 *   - string_prepend        : put a C string in front by building a new string then tacking on old content
 *   - string_clear          : reset to "", but keep the allocated memory
 *   - string_exactly        : replace current contents with a copy of the given C string
 *
 * Memory notes:
 *   - malloc/calloc allocate memory on the heap
 *   - realloc resizes a previous allocation
 *   - Always keep capacity >= length + 1 so we have room for the trailing '\0'
 */

#include "stringer.h"
#include <stdlib.h>
#include <string.h>

/**
 * Allocate a new string with an empty string "".
 * buffer size is 1 so it can store the '\0' terminator.
 */
struct string *string_new(void)
{
    // allocate the struct
    struct string *mystring = malloc(sizeof(struct string));
    if (!mystring) {
        return NULL;
    }

    // allocate 1 byte and set it to 0 so buffer holds ""
    mystring->buffer   = calloc(1, sizeof(char));
    if (!mystring->buffer) {
        free(mystring);
        return NULL;
    }

    mystring->capacity = 1;  // can store just the '\0'
    mystring->length   = 0;  // empty string
    return mystring;
}

/**
 * Allocate a new string by copying it from `cstr`.
 * Uses strdup to allocate and copy the bytes (including '\0').
 */
struct string *string_new_from(const char *cstr)
{
    struct string *ret = malloc(sizeof(struct string));
    if (!ret) {
        return NULL;
    }

    ret->buffer = strdup(cstr);   // copies bytes plus trailing '\0'
    if (!ret->buffer) {
        free(ret);
        return NULL;
    }

    ret->length   = strlen(cstr); // number of visible characters
    ret->capacity = ret->length + 1; // account for '\0'
    return ret;
}

/**
 * Reallocate the capacity of a string. Does not change the
 * string contents except when shrinking would chop data.
 * If `capacity` is <= current capacity, we do nothing.
 * If after resizing capacity <= length, we clamp length to capacity - 1
 * and write the '\0' terminator at buffer[length].
 *
 * Important safety fix:
 *   Use a temporary pointer for realloc so we do not lose the original
 *   buffer if realloc fails.
 */
void string_realloc(struct string *s, size_t capacity)
{
    if (!s) return;

    // only grow
    if (capacity > s->capacity) {
        void *tmp = realloc(s->buffer, capacity);
        if (!tmp) {
            // realloc failed, keep old buffer and capacity unchanged
            return;
        }
        s->buffer   = tmp;
        s->capacity = capacity;
    }

    // if capacity is less than or equal to length, clamp length
    if (s->capacity <= s->length) {
        s->length = s->capacity - 1;          // leave room for '\0'
        s->buffer[s->length] = '\0';          // terminator at index length
    }
}

/**
 * Frees all allocated memory for a string.
 */
void string_free(struct string *s)
{
    if (s) {
        free(s->buffer);
        free(s);
    }
}

// Utilities
/**
 * Pushes a single character to the end of the string.
 * Ensures there is space for the new character and the trailing '\0'.
 */
void string_push(struct string *s, char c)
{
    if (!s) return;

    // need length + 1 (for new char) + 1 (for '\0') <= capacity
    if (s->length + 1 >= s->capacity) {
        // grow by at least 1 byte. Simple and OK for small workloads.
        // For performance later, you could grow by a factor (like double).
        string_realloc(s, s->capacity + 1);
        // If realloc failed, capacity did not grow. Guard against overflow.
        if (s->length + 1 >= s->capacity) {
            return; // out of memory; do nothing
        }
    }

    s->buffer[s->length] = c;   // write the character
    s->length++;                // increase logical length
    s->buffer[s->length] = '\0';// keep it NUL-terminated
}

/**
 * Appends a C-style string to the end.
 * Pushes each character so the string stays well-formed.
 */
void string_append(struct string *s, const char *cstr)
{
    if (!s || !cstr) return;

    // minor efficiency: compute strlen once
    size_t n = strlen(cstr);
    for (size_t i = 0; i < n; i++) {
        string_push(s, cstr[i]);
    }
}

/**
 * Prepends a C-style string to the front.
 * Strategy:
 *   1) Make a new string from cstr
 *   2) Append the original contents to that
 *   3) Move the new buffer into s and free the temporary struct
 */
void string_prepend(struct string *s, const char *cstr)
{
    if (!s || !cstr) return;

    struct string *prepend = string_new_from(cstr);
    if (!prepend) return;

    // append old content to the end of "prepend"
    for (size_t i = 0; i < s->length; i++) {
        string_push(prepend, s->buffer[i]);
    }

    // transfer ownership of buffer into s
    free(s->buffer);
    s->buffer   = prepend->buffer;
    s->length   = prepend->length;
    s->capacity = prepend->capacity;

    // free only the struct shell; do not free the buffer we just moved
    free(prepend);
}

/**
 * Clears the string to empty "".
 * Does not free any memory. Keeps capacity for future growth.
 */
void string_clear(struct string *s)
{
    if (!s) return;
    s->length     = 0;
    s->buffer[0]  = '\0';
}

/**
 * Sets the string exactly to the given C-string `cstr`.
 * Replaces current buffer with a fresh copy of cstr.
 */
void string_exactly(struct string *s, const char *cstr)
{
    if (!s || !cstr) return;

    struct string *exactly = string_new_from(cstr);
    if (!exactly) return;

    // move the freshly allocated contents into s
    free(s->buffer);
    s->buffer   = exactly->buffer;
    s->length   = exactly->length;
    s->capacity = exactly->capacity;

    // free only the struct shell
    free(exactly);
}
