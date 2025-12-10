#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_MAX_LEN 32

typedef struct {
    char name[NAME_MAX_LEN + 1];
    double score;
} Entry;

static int cmp_entry(const Entry *a, const Entry *b) {
    if (a->score != b->score) return (b->score < a->score) ? -1 : 1;
    return 0;
}

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

    char filename[256];
    snprintf(filename, sizeof(filename), "%s.txt", argv[1]);
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror(filename);
        return 1;
    }

    size_t cap = 32, n = 0;
    Entry *a = malloc(cap * sizeof *a);
    if (!a) { perror("malloc"); fclose(f); return 1; }

    char namebuf[NAME_MAX_LEN + 1];
    double score;

    while (fscanf(f, "%32s %lf", namebuf, &score) == 2) {
        if (n == cap) {
            cap *= 2;
            Entry *p = realloc(a, cap * sizeof *a);
            if (!p) { perror("realloc"); free(a); fclose(f); return 1; }
            a = p;
        }
        a[n].score = score;
        n++;
    }
    fclose(f);

    if (n == 0) {
        free(a);
        return 0;
    }

    insertion_sort(a, n);

    for (size_t i = 0; i < n; i++) {
        printf("%.2f\n", a[i].score);
    }

    free(a);
    return 0;
}
