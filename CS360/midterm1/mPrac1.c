/*
Instructions:
Take the name of a file as the first user-supplied command line argument.
The file contains an unknown number of words (one per line).
Your job is to read all the words and produce the following statistics in this order:

Count – the total number of words.

Shortest – the shortest word in the file.

Longest – the longest word in the file.

First – the word that comes first in dictionary (lexicographic) order.

Last – the word that comes last in dictionary order.

Palindrome Count – how many words are palindromes (e.g., "level", "racecar").

Requirements:

Words can be assumed to be no longer than 50 characters.

You must store the words in a dynamically growing array (use malloc/realloc).

Lexicographic comparisons must be case-sensitive (use strcmp).

A palindrome check must be case-sensitive.

Format output so the labels are in a left-justified field 15 characters wide followed by =, similar to the integer problem.

Example Output:

Count           = 15
Shortest        = a
Longest         = encyclopedia
First           = Apple
Last            = zebra
Palindrome Count= 3
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_WORD_LEN 50

static int is_palindrome(const char *s) {
    size_t i = 0, j = strlen(s);
    if (j == 0) return 1;
    j--;
    while (i < j) {
        if (s[i] != s[j]) return 0;
        i++; j--;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }

    size_t capacity = 32;
    size_t count = 0;
    char **words = malloc(capacity * sizeof *words);
    if (!words) {
        perror("malloc");
        fclose(f);
        return 1;
    }

    char buf[MAX_WORD_LEN + 2];  // +2 to safely hold newline and NUL
    while (fgets(buf, sizeof buf, f)) {
        // Strip trailing newline
        size_t n = strcspn(buf, "\r\n");
        buf[n] = '\0';

        // Skip empty lines
        if (buf[0] == '\0') continue;

        // Enforce max length (truncate if a line exceeded buffer; fgets already did)
        // If you want to reject overly long lines instead, you could check if
        // the last read char was not newline and then read the rest of the line.

        if (count == capacity) {
            capacity *= 2;
            char **p = realloc(words, capacity * sizeof *words);
            if (!p) {
                perror("realloc");
                // free what we have so far
                for (size_t i = 0; i < count; i++) free(words[i]);
                free(words);
                fclose(f);
                return 1;
            }
            words = p;
        }

        words[count] = strdup(buf);
        if (!words[count]) {
            perror("strdup");
            for (size_t i = 0; i < count; i++) free(words[i]);
            free(words);
            fclose(f);
            return 1;
        }
        count++;
    }
    fclose(f);

    // If no words, print zeros and blanks as appropriate
    if (count == 0) {
        printf("%-15s = %zu\n", "Count", (size_t)0);
        printf("%-15s = %s\n", "Shortest", "");
        printf("%-15s = %s\n", "Longest", "");
        printf("%-15s = %s\n", "First", "");
        printf("%-15s = %s\n", "Last", "");
        printf("%-15s = %d\n", "Palindrome Count", 0);
        free(words);
        return 0;
    }

    // Initialize stats with the first word
    size_t idx_short = 0, idx_long = 0, idx_first = 0, idx_last = 0;
    int pal_count = 0;

    // Count palindromes for the first word
    pal_count += is_palindrome(words[0]) ? 1 : 0;

    for (size_t i = 1; i < count; i++) {
        size_t len_i = strlen(words[i]);
        size_t len_short = strlen(words[idx_short]);
        size_t len_long = strlen(words[idx_long]);

        if (len_i < len_short) idx_short = i;
        if (len_i > len_long)  idx_long = i;

        if (strcmp(words[i], words[idx_first]) < 0) idx_first = i;
        if (strcmp(words[i], words[idx_last])  > 0) idx_last  = i;

        pal_count += is_palindrome(words[i]) ? 1 : 0;
    }

    // Output
    printf("%-15s = %zu\n", "Count", count);
    printf("%-15s = %s\n", "Shortest", words[idx_short]);
    printf("%-15s = %s\n", "Longest", words[idx_long]);
    printf("%-15s = %s\n", "First", words[idx_first]);
    printf("%-15s = %s\n", "Last", words[idx_last]);
    printf("%-15s = %d\n", "Palindrome Count", pal_count);

    // Cleanup
    for (size_t i = 0; i < count; i++) free(words[i]);
    free(words);
    return 0;
}


