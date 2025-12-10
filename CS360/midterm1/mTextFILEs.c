/*
 * Program Description
 * -------------------
 * What this program does:
 *   Reads a text file whose lines each contain three integers: a b c
 *   For every line, it computes the expression: a * b + c
 *   and prints a nicely formatted line showing the math and the result.
 *
 * How it works, in plain steps:
 *   1) You pass the input filename as the first command-line argument.
 *   2) The program opens the file for reading.
 *   3) It repeatedly uses fscanf to read three integers (a, b, c) per line.
 *   4) For each triple, it prints: "a * b + c = result" with tidy spacing.
 *   5) When input ends or a malformed line is found, the loop stops and the file is closed.
 *
 * Notes:
 *   - The fscanf format "%d %d %d" expects three space-separated integers on each line.
 *   - If your data can exceed int range, consider using long long and "%lld".
 *   - The formatting "%-4d" means left-justify the number in a 4-character field.
 */

#include <stdio.h>

int main(int argc, char* argv[]) 
{
    FILE *fl;
    int a, b, c;

    // Require a filename argument: argv[1]
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    // Open the file for reading text
    fl = fopen(argv[1], "r");
    if (!fl) {
        // perror prints a system-provided error message (e.g., "No such file or directory")
        perror(argv[1]);
        return 2;
    }

    // Read triples "a b c" until fscanf stops matching 3 integers
    while (fscanf(fl, "%d %d %d", &a, &b, &c) == 3) {
        // %-4d = left-justify in width 4 so columns line up nicely
        printf("%-4d * %-4d + %-4d = %d\n", a, b, c, a * b + c);
    }

    fclose(fl);
    return 0; // success
}
