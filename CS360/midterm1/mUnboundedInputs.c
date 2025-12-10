/*
 * Program Description
 * -------------------
 * What this program does:
 *   Reads a bunch of names (one token at a time) from standard input,
 *   stores them in a growable array, sorts them alphabetically, and prints them.
 *
 * How it works in plain steps:
 *   1) Read up to 10 characters per name using scanf("%10s", ...).
 *   2) Keep the names in a dynamic array of char* (each name is strdup'ed).
 *      - Start with a small capacity and grow (realloc) when full.
 *   3) Sort with qsort and a comparator that uses strcmp.
 *   4) Print the sorted names and free all allocated memory.
 *
 * Key functions:
 *   - malloc/realloc/free: manage heap memory for the pointer array
 *   - strdup: allocate + copy each input string
 *   - qsort: generic sort; uses our compare() to order strings via strcmp
 *   - scanf("%10s"): reads at most 10 chars + terminator, avoiding overflow
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Comparator used by qsort for an array of char*.
// qsort gives us pointers to the array elements (each element is char*),
// so the incoming pointers are (const char **). We dereference both and call strcmp.
int compare(const void *a, const void *b) {
    const char *sa = *(const char * const *)a;
    const char *sb = *(const char * const *)b;
    return strcmp(sa, sb);
}

int main(void)
{
    // Dynamic array of pointers-to-char (each points to one name string)
    size_t capacity = 8;                     // start small; grow as needed
    size_t count    = 0;                     // how many names we actually have
    char  **names   = malloc(capacity * sizeof *names);
    if (!names) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    // Buffer to read each input token. +1 for the trailing '\0'.
    char input[11];

    // Read names until EOF. "%10s" limits to 10 chars to avoid overflow.
    while (scanf("%10s", input) == 1) {
        // Ensure capacity: if full, double it.
        if (count == capacity) {
            size_t new_capacity = capacity ? capacity * 2 : 8;
            char **tmp = realloc(names, new_capacity * sizeof *names);
            if (!tmp) {
                fprintf(stderr, "realloc failed\n");
                // Free what we already allocated before bailing.
                for (size_t i = 0; i < count; i++) free(names[i]);
                free(names);
                return 2;
            }
            names = tmp;
            capacity = new_capacity;
        }

        // Duplicate the input string onto the heap.
        char *copy = strdup(input);
        if (!copy) {
            fprintf(stderr, "strdup failed\n");
            for (size_t i = 0; i < count; i++) free(names[i]);
            free(names);
            return 3;
        }

        names[count++] = copy;
    }

    // Sort the array of strings alphabetically.
    if (count > 1) {
        qsort(names, count, sizeof *names, compare);
    }

    // Print results and free each string.
    for (size_t i = 0; i < count; i++) {
        printf("%s\n", names[i]);
        free(names[i]);
    }

    // Free the pointer array itself.
    free(names);

    return 0;
}
