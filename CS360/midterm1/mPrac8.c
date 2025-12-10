/* Problem: Leaderboard Builder

Input:
Take the filename as the first command-line argument.
The file has lines with a name and a score separated by whitespace:

name is a single word, up to 32 chars.

score is an integer (can be negative, 32-bit).

Example file:

Alice 95
Bob 95
carol 88
dave 100
Eve 88


Task:
Read all records, store them in a growable array, then sort them by:

score descending

name ascending (case-sensitive) on ties

You must implement your own sorting algorithm (for example, insertion sort or merge sort). Do not use qsort.

Output:
Print a leaderboard with dense ranks (ties share the same rank; next rank does not skip numbers).
Fields are left-justified: Rank width 6, Name width 20, Score width 6.

Format:

Rank   Name                 Score 
1      dave                 100   
2      Alice                95    
2      Bob                  95    
3      Eve                  88    
3      carol                88    


Requirements:

Use malloc and realloc to grow the array of records.

Implement your own sort. State which one you used in a comment.

Ranking is dense:

If scores are [100, 95, 95, 88, 88], ranks are [1, 2, 2, 3, 3].

Handle empty files:

Print only the header row, then exit.

Handle malformed lines robustly:

Skip a line if it does not match name score.

Header row to print first:

Rank   Name                 Score 


Hints you can rely on:

Insertion sort is short and great for exams:

Iterate i = 1..n-1, extract key, shift larger items right, insert.

For ranking after sorting:

First item rank = 1.

For each next item:

If score[i] == score[i-1], same rank.

Else rank = previous rank + 1.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_MAX_LEN 32

typedef struct {
    char name[NAME_MAX_LEN + 1]; // up to 32 chars + NUL
    int  score;
} Entry;

/* Compare for leaderboard order:
   - score descending
   - name ascending (case-sensitive) on ties
   Return >0 if a should come AFTER b, <0 if BEFORE, 0 if equal. */
static int cmp_entry(const Entry *a, const Entry *b) {
    if (a->score != b->score) return (b->score - a->score); // desc
    return strcmp(a->name, b->name);                         // asc
}

/* Insertion sort (stable enough for this usage) */
static void insertion_sort(Entry *arr, size_t n) {
    for (size_t i = 1; i < n; i++) {
        Entry key = arr[i];
        size_t j = i;
        while (j > 0 && cmp_entry(&arr[j - 1], &key) > 0) {
            arr[j] = arr[j - 1];
            j--;
        }
        arr[j] = key;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) { perror(argv[1]); return 1; }

    size_t cap = 32, n = 0;
    Entry *a = malloc(cap * sizeof *a);
    if (!a) { perror("malloc"); fclose(f); return 1; }

    char namebuf[NAME_MAX_LEN + 1];
    int score;

    /* Read lines like: "<name> <score>" ; skip malformed lines */
    while (fscanf(f, "%32s %d", namebuf, &score) == 2) {
        if (n == cap) {
            cap *= 2;
            Entry *p = realloc(a, cap * sizeof *a);
            if (!p) { perror("realloc"); free(a); fclose(f); return 1; }
            a = p;
        }
        strncpy(a[n].name, namebuf, NAME_MAX_LEN);
        a[n].name[NAME_MAX_LEN] = '\0'; // ensure NUL
        a[n].score = score;
        n++;
    }
    fclose(f);

    /* Header always printed */
    printf("%-6s %-20s %-6s\n", "Rank", "Name", "Score");

    if (n == 0) {
        free(a);
        return 0;
    }

    insertion_sort(a, n);

    /* Dense ranks */
    int current_rank = 1;
    printf("%-6d %-20s %-6d\n", current_rank, a[0].name, a[0].score);
    for (size_t i = 1; i < n; i++) {
        if (a[i].score != a[i - 1].score) {
            current_rank++; // next distinct score gets next rank
        }
        printf("%-6d %-20s %-6d\n", current_rank, a[i].name, a[i].score);
    }

    free(a);
    return 0;
}
