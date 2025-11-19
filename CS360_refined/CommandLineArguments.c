#include <stdio.h>

int main (int argc, char *argv[]) {
    int a, b, c;
    if (argc < 4) {
        printf("Usage: %s <a> <b> <c>\n", argv[0]);
        return 1;
    }
    if (1 != sscanf(argv[1], "%d", &a)) {
        printf("Invalid value for a\n");
        return 2;
    }
    if (1 != sscanf(argv[2], "%d", &b)) {
        printf("Invalid value for b\n");
        return 3;
    }
    if (1 != sscanf(argv[3], "%d", &c)) {
        printf("Invalid value for c\n");
        return 4;
    }
    double Result = (double)a * b / c;
    printf("Result = %.3lf\n", Result);
    return 0;
}
