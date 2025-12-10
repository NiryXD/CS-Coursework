/*
Reverse File Content

Instructions:
Take the name of a file as input. The file contains integers (one per line).
Read them all into memory, then print them in reverse order to stdout, one per line.

Example Input:

10
20
30
40


Example Output:

40
30
20
10
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

    size_t cap = 32, n = 0;
    int *a = malloc(cap * sizeof *a);
    if (!a) { perror("malloc"); fclose(f); return 1; }

    int v;
    while (fscanf(f, "%d", &v) == 1) {
        if (n == cap) {
            cap *= 2;
            int *p = realloc(a, cap * sizeof *a);
            if (!p) { perror("realloc"); free(a); fclose(f); return 1; }
            a = p;
        }
        a[n++] = v;
    }
    fclose(f);

    for (size_t i = n; i > 0; i--) {
        printf("%d\n", a[i - 1]);
    }
    free(a);
    return 0;
}
