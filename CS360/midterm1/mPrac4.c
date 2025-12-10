/*
Unique Word Counter

Instructions:
The input file contains many words (whitespace-separated).
You must count how many unique words there are. Case-sensitive.

Steps:

Read all words into a dynamic array of strings.

Before adding a new word, check if it already exists.

At the end, print the total word count and the unique word count.

Example Output:

Total Words  = 20
Unique Words = 15
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int find_word(char **arr, int n, const char *w) {
    for (int i = 0; i < n; i++) if (strcmp(arr[i], w) == 0) return i;
    return -1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "r");
    if (!f) { perror(argv[1]); return 1; }

    size_t cap = 32, uniq = 0;
    char **words = malloc(cap * sizeof *words);
    if (!words) { perror("malloc"); fclose(f); return 1; }

    long long total = 0;
    char buf[128];

    while (fscanf(f, "%127s", buf) == 1) {
        total++;
        if (find_word(words, (int)uniq, buf) == -1) {
            if (uniq == cap) {
                cap *= 2;
                char **p = realloc(words, cap * sizeof *words);
                if (!p) {
                    perror("realloc");
                    for (size_t i = 0; i < uniq; i++) free(words[i]);
                    free(words);
                    fclose(f);
                    return 1;
                }
                words = p;
            }
            words[uniq] = strdup(buf);
            if (!words[uniq]) {
                perror("strdup");
                for (size_t i = 0; i < uniq; i++) free(words[i]);
                free(words);
                fclose(f);
                return 1;
            }
            uniq++;
        }
    }
    fclose(f);

    printf("Total Words  = %lld\n", total);
    printf("Unique Words = %zu\n", uniq);

    for (size_t i = 0; i < uniq; i++) free(words[i]);
    free(words);
    return 0;
}
