/*
Instructions:
The input file contains lines with name grade where:

name is a single word (max 20 chars).

grade is an integer from 0–100.

Your job is to:

Print the total number of students.

Print the average grade to 2 decimals.

Print the highest grade and the student who got it.

Print the lowest grade and the student who got it.

Output example:

Count       = 12
Average     = 78.25
Highest     = Alice (100)
Lowest      = Bob (54)
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

    char name[64];
    int grade;

    long long sum = 0;
    int count = 0;
    int have = 0;

    char best_name[64] = "";
    char worst_name[64] = "";
    int best = 0, worst = 0;

    while (fscanf(f, "%63s %d", name, &grade) == 2) {
        sum += grade;
        count++;
        if (!have) {
            have = 1;
            best = worst = grade;
            strcpy(best_name, name);
            strcpy(worst_name, name);
        } else {
            if (grade > best) { best = grade; strcpy(best_name, name); }
            if (grade < worst){ worst = grade; strcpy(worst_name, name); }
        }
    }
    fclose(f);

    if (count == 0) {
        printf("Count       = 0\n");
        printf("Average     = 0.00\n");
        printf("Highest     =  (0)\n");
        printf("Lowest      =  (0)\n");
        return 0;
    }

    double avg = (double)sum / count;

    printf("Count       = %d\n", count);
    printf("Average     = %.2f\n", avg);
    printf("Highest     = %s (%d)\n", best_name, best);
    printf("Lowest      = %s (%d)\n", worst_name, worst);
    return 0;
}
