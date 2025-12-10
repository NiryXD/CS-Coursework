/*
1. Character Frequency Counter

Instructions:
Take the name of a file as the first command-line argument. The file contains plain text (letters, digits, punctuation, etc.).
Your job is to count how many times each lowercase letter (‘a’–‘z’) appears. Ignore uppercase and non-letter characters.

Output format:
Print the counts in alphabetical order, one per line, label left-justified in a 5-character field:

a    = 15
b    = 2
c    = 0
...
z    = 3
*/

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }
    FILE *f = fopen(argv[1], "r");
    if (!f) { perror(argv[1]); return 1; }

    long long counts[26] = {0};
    int ch;
    while ((ch = fgetc(f)) != EOF) {
        if (ch >= 'a' && ch <= 'z') counts[ch - 'a']++;
        /* uppercase and non-letters are ignored by design */
    }
    fclose(f);

    for (int i = 0; i < 26; i++) {
        printf("%-5c= %lld\n", 'a' + i, counts[i]);
    }
    return 0;
}
