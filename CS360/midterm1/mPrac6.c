/*
Longest Line Finder

Instructions:
The file contains lines of arbitrary length (≤ 200 chars).
You must:

Count how many lines there are.

Find the longest line and print it.

Print its length (character count).

Output Example:

Line Count   = 6
Longest Line = The quick brown fox jumps over the lazy dog
Length       = 43
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "r");
    if (!f) { perror(argv[1]); return 1; }

    char buf[202]; /* lines up to 200 chars + newline + NUL */
    long long line_count = 0;
    char longest[202] = "";
    size_t longest_len = 0;

    while (fgets(buf, sizeof buf, f)) {
        line_count++;
        size_t n = strcspn(buf, "\r\n");
        buf[n] = '\0';
        if (n > longest_len) {
            longest_len = n;
            strcpy(longest, buf);
        }
    }
    fclose(f);

    printf("Line Count   = %lld\n", line_count);
    printf("Longest Line = %s\n", longest);
    printf("Length       = %zu\n", longest_len);
    return 0;
}
