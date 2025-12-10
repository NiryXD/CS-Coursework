/*
 * Program Description
 * -------------------
 * What this program does:
 *   Reads three integers a, b, c from the command line,
 *   computes Result = (a * b) / c as a floating-point number,
 *   then prints the result with three digits after the decimal.
 *
 * How it works, in plain steps:
 *   1) Checks that you provided 3 arguments after the program name.
 *   2) Uses sscanf to convert the strings argv[1], argv[2], argv[3] into ints a, b, c.
 *      - If any conversion fails, it prints a message and exits with a code.
 *   3) Casts a to double before multiplying by b and dividing by c so the math is done
 *      in floating point, not integer math. This avoids truncation.
 *   4) Prints the result using printf with formatting: "%.3lf" means 3 digits after the decimal.
 *
 * Notes:
 *   - If c is 0, dividing by zero would be invalid. We check and exit with an error.
 *   - Return codes:
 *       1: wrong number of arguments
 *       2: could not parse a
 *       3: could not parse b
 *       4: could not parse c
 *       5: division by zero attempted
 */

#include <stdio.h>

int main (int argc, char *argv[]) {
    int a, b, c;

    // Need exactly 3 user-supplied arguments: a, b, c
    if (argc < 4) {
        printf("Usage: %s <a> <b> <c>\n", argv[0]);
        return 1;
    }

    // Convert argv[1] into integer a
    if (1 != sscanf(argv[1], "%d", &a)) {
        printf("Invalid value for a\n");
        return 2;
    }
    // Convert argv[2] into integer b
    if (1 != sscanf(argv[2], "%d", &b)) {
        printf("Invalid value for b\n");
        return 3;
    }
    // Convert argv[3] into integer c
    if (1 != sscanf(argv[3], "%d", &c)) {
        printf("Invalid value for c\n");
        return 4;
    }

    // Guard against division by zero
    if (c == 0) {
        printf("Division by zero error: c must not be 0\n");
        return 5;
    }

    // Cast to double so the arithmetic is floating point, not integer
    double Result = (double)a * b / c;

    // Print with 3 digits after the decimal point
    printf("Result = %.3lf\n", Result);

    return 0;
}
