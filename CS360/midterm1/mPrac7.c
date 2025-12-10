/*
Take the name of a file as the first user-supplied command line argument.
The file contains floating-point numbers (one per line), each representing a temperature reading.

Your job is to:

Count the total number of readings.

Sum all the readings.

Average (to 2 decimal places).

Minimum temperature.

Maximum temperature.

Standard Deviation (to 2 decimals).

Output Format:
Labels left-justified in a 12-character field, followed by =, like so:

Count       = 100
Sum         = 3456.78
Average     = 34.57
Min         = 12.30
Max         = 54.60
StdDev      = 8.21
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) { perror(argv[1]); return 1; }

    // We'll compute stats in one pass with Welford's algorithm
    // for numerical stability: running mean and M2 (sum of squared diffs).
    long long count = 0;
    double sum = 0.0;
    double mean = 0.0;
    double M2 = 0.0;
    double v;              // current reading
    int have_minmax = 0;
    double minv = 0.0, maxv = 0.0;

    while (fscanf(f, "%lf", &v) == 1) {
        count++;
        sum += v;

        if (!have_minmax) {
            minv = maxv = v;
            have_minmax = 1;
        } else {
            if (v < minv) minv = v;
            if (v > maxv) maxv = v;
        }

        // Welford update
        double delta = v - mean;
        mean += delta / count;
        double delta2 = v - mean;
        M2 += delta * delta2;
    }
    fclose(f);

    double avg = (count > 0) ? mean : 0.0;
    double stddev = (count > 0) ? sqrt(M2 / (double)count) : 0.0;

    // Output with labels left-justified in 12-character field, then " = "
    printf("%-12s = %lld\n", "Count", count);
    printf("%-12s = %.2f\n", "Sum", sum);
    printf("%-12s = %.2f\n", "Average", avg);
    printf("%-12s = %.2f\n", "Min", have_minmax ? minv : 0.0);
    printf("%-12s = %.2f\n", "Max", have_minmax ? maxv : 0.0);
    printf("%-12s = %.2f\n", "StdDev", stddev);

    return 0;
}
