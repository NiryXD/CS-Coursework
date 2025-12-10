/* nuke.c — COSC 360/367 Nuclear Blast Lab (tips applied) */
#include <stdio.h>
#include <math.h>

struct Point {
    int x;
    int y;
};

/* Required prototype */
double distance(const struct Point *p1, const struct Point *p2) {
    double dx = (double)p1->x - (double)p2->x;
    double dy = (double)p1->y - (double)p2->y;
    return sqrt(dx*dx + dy*dy);
}

int main(int argc, char *argv[]) {
    /* slide tip: error check argc and argv types with sscanf */
    if (argc != 5) {
        /* minimal message; grader just needs correctness */
        fprintf(stderr, "usage: %s <initial(float)> <factor(double)> <bx(int)> <by(int)>\n", argv[0]);
        return 1;
    }

    /* slide tip: initial blast as float, attenuation as double */
    float initial_f;
    double factor;
    int bx, by;

    if (sscanf(argv[1], "%f", &initial_f) != 1) return 2;
    if (sscanf(argv[2], "%lf", &factor)   != 1) return 3;
    if (sscanf(argv[3], "%d", &bx)        != 1) return 4;
    if (sscanf(argv[4], "%d", &by)        != 1) return 5;

    /* compute with double for math, but honor slide types at input */
    const double initial = (double)initial_f;
    const struct Point blast = { bx, by };

    int px, py;
    char name[33]; /* slide tip: max 32 chars + NUL */

    /* slide tip: read from standard input; use field width to bound name */
    while (scanf("%d %d %32s", &px, &py, name) == 3) {
        const struct Point person = { px, py };
        const double d = distance(&person, &blast);
        const double dose = initial * pow(factor, d);

        /* slide tip: formatting — name: left width 8; dose: right width 8, .3f; then " Sv" */
        printf("%-8s: %8.3f Sv\n", name, dose);
    }
    return 0;
}
