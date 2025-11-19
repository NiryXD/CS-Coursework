#include "stringer.h"
#include <stdlib.h>
#include <string.h>

/**
 * Allocate a new string with an empty string ""
 */
struct string *string_new(void)
{
    struct string *mystring = malloc(sizeof(struct string));
    mystring->buffer = calloc(1, sizeof(char));
    mystring->capacity = 1;
    mystring->length = 0;
    return mystring;
}
/**
 * Allocate a new string by copying it from `cstr`
 */
struct string *string_new_from(const char *cstr)
{
    struct string *ret;

    ret = malloc(sizeof(struct string));
    ret->buffer = strdup(cstr);
    ret->length = strlen(cstr);
    ret->capacity = ret->length + 1;
    return ret;
}
/**
 * Reallocate the capacity of a string. Does not change the
 * string itself, but just allocates new memory. If `capacity`
 * is <= length, then length is set to capacity - 1.
 */
void string_realloc(struct string *s, size_t capacity)
{
    if (capacity > s->capacity){
        s->buffer = realloc(s->buffer, capacity);
        s->capacity = capacity;
    }
    if (s->capacity <= s->length) {
        s->length = s->capacity - 1;
        s->buffer[s->length - 1] = '\0';
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
 * Pushes a single charater to the string.
 */
void string_push(struct string *s, char c)
{
    if (s->length + 1 >= s->capacity) { 
        string_realloc(s, s->capacity + 1);
    }

    s->buffer[s->length] = c;
    s->length++;
    s->buffer[s->length] = '\0';


    
}
/**
 * Appends a C-style string to the string. The new string
 * is a combination of both the old string and `cstr`.
 */
void string_append(struct string *s, const char *cstr)
{
    for (int i = 0; i <strlen(cstr); i++) {
        string_push(s, cstr[i]);
    }
}
/**
 * Prepends a C-style string to the string. The new string
 * is a combination of both the old string and `cstr`.
 */
void string_prepend(struct string *s, const char *cstr)
{
    struct string *prepend = string_new_from(cstr);

    int i = 0;
    while (i < s->length) {
        char letter = s->buffer[i];
        string_push(prepend, letter);
        i++;
    }

    free(s->buffer);
    s->buffer = prepend->buffer;
    s->length = prepend->length;
    s->capacity = prepend->capacity;

    free(prepend);
    

    
}
/**
 * Clears the string to empty string "". Does not free any memory.
 */
void string_clear(struct string *s)
{
    s->length = 0;
    s->buffer[0] = '\0';
}
/**
 * Sets the string exactly to the given C-string `cstr`.
 */
void string_exactly(struct string *s, const char *cstr)
{
    struct string *exactly = string_new_from(cstr);

    free(s->buffer);
    s->buffer = exactly->buffer;
    s->length = exactly->length;
    s->capacity = exactly->capacity;

    free(exactly);
}
