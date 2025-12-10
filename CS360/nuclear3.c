#include <stdio.h>
#include <math.h>

struct Point { int x; int y; };

double distance(const struct Point *p1, const struct Point *p2) {
    double dx = (double)p1->x - (double)p2->x;
    double dy = (double)p1->y - (double)p2->y;
    return sqrt(dx*dx + dy*dy);
}

int main(int argc, char *argv[]) {
    if (argc != 5) return 1;
    double initial, factor;
    int bx, by;
    if (sscanf(argv[1], "%lf", &initial) != 1) return 1;
    if (sscanf(argv[2], "%lf", &factor)  != 1) return 1;
    if (sscanf(argv[3], "%d", &bx)       != 1) return 1;
    if (sscanf(argv[4], "%d", &by)       != 1) return 1;

    struct Point blast = { bx, by };
    int px, py; char name[33];
    while (scanf("%d %d %32s", &px, &py, name) == 3) {
        struct Point person = { px, py };
        double dose = initial * pow(factor, distance(&person, &blast));
        printf("%-8s: %8.3f Sv\n", name, dose);
    }
    return 0;
}
