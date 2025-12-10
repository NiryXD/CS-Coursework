// sort_doubles_optional_txt.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void insertion_sort_desc(double *a, size_t n) {
    for (size_t i = 1; i < n; ++i) {
        double key = a[i];
        size_t j = i;
        while (j > 0 && a[j - 1] < key) {
            a[j] = a[j - 1];
            --j;
        }
        a[j] = key;
    }
}

static void trim(char *s) {
    // left trim
    size_t i = 0, j = 0;
    while (s[i] && isspace((unsigned char)s[i])) i++;
    if (i) memmove(s, s + i, strlen(s + i) + 1);
    // right trim
    size_t len = strlen(s);
    while (len && isspace((unsigned char)s[len - 1])) s[--len] = '\0';
    // strip surrounding quotes
    if (len >= 2 && ((s[0] == '"' && s[len - 1] == '"') || (s[0] == '\'' && s[len - 1] == '\''))) {
        memmove(s, s + 1, len - 2);
        s[len - 2] = '\0';
    }
}

static int has_extension(const char *s) {
    // return 1 if there's a '.' after the last path separator
    const char *slash1 = strrchr(s, '/');
#ifdef _WIN32
    const char *slash2 = strrchr(s, '\\');
    if (!slash1 || (slash2 && slash2 > slash1)) slash1 = slash2;
#endif
    const char *base = slash1 ? slash1 + 1 : s;
    const char *dot  = strrchr(base, '.');
    return dot != NULL && dot != base; // treat ".hidden" as having an extension too
}

int main(void) {
    char input[256];

    // Read filename token/line from stdin (works with or without prompt)
    // If your grader forbids prompts, comment the next line.
    // printf("Enter filename: ");
    if (!fgets(input, sizeof input, stdin)) {
        fprintf(stderr, "Failed to read filename from stdin\n");
        return 1;
    }
    trim(input);
    if (input[0] == '\0') {
        fprintf(stderr, "Empty filename\n");
        return 1;
    }

    // Try as-typed first
    FILE *f = fopen(input, "r");

    // If open failed and no extension was provided, try adding ".txt"
    char fallback[256];
    if (!f && !has_extension(input)) {
        size_t in_len = strlen(input);
        if (in_len + 4 < sizeof fallback) {
            memcpy(fallback, input, in_len);
            memcpy(fallback + in_len, ".txt", 5); // includes '\0'
            f = fopen(fallback, "r");
        }
    }

    if (!f) {
        perror(input);
        return 1;
    }

    // Dynamic array for doubles
    size_t cap = 32, n = 0;
    double *vals = (double *)malloc(cap * sizeof *vals);
    if (!vals) { perror("malloc"); fclose(f); return 1; }

    // Read doubles (any whitespace-separated format)
    double x;
    while (fscanf(f, "%lf", &x) == 1) {
        if (n == cap) {
            cap *= 2;
            double *p = (double *)realloc(vals, cap * sizeof *p);
            if (!p) { perror("realloc"); free(vals); fclose(f); return 1; }
            vals = p;
        }
        vals[n++] = x;
    }
    fclose(f);

    if (n) {
        insertion_sort_desc(vals, n);
        for (size_t i = 0; i < n; ++i) {
            printf("%.2f\n", vals[i]);
        }
    }

    free(vals);
    return 0;
}
