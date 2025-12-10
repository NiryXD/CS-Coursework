/*
 * Program Description
 * -------------------
 * What this program does:
 *   Opens a binary file that contains 32-bit signed integers back to back,
 *   reads them one at a time, sums them into a 64-bit total, and prints the sum.
 *
 * How it works in plain steps:
 *   1) You pass the file name as the first command-line argument.
 *   2) The program opens the file in binary read mode "rb".
 *   3) It repeatedly calls fread to read one int at a time.
 *   4) Each int is added to a long long accumulator.
 *   5) When fread stops returning items, it closes the file and prints the sum.
 *
 * Notes:
 *   - fread returns the number of items read. Here we ask for 1 item of sizeof(int).
 *   - This assumes sizeof(int) is 4 bytes on your platform and endianness matches the file.
 *   - If your data can exceed int range or you need cross-platform safety, consider fixed-width types.
 */

#include <stdio.h>

int main(int argc, char *argv[])
{
    // Require exactly one argument: the file name
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    // Open the file in binary mode
    FILE *f = fopen(argv[1], "rb");
    if (f == NULL) {
        perror(argv[1]); // prints a system error message like "No such file or directory"
        return 2;
    }

    int value = 0;          // will hold each 32-bit signed integer read from file
    long long sum = 0;      // 64-bit total so we do not overflow easily

    // Read integers until fread stops returning 1 item
    while (fread(&value, sizeof(int), 1, f) == 1) {
        sum += value;       // add current integer to running total
    }

    fclose(f);              // always close the file when done

    // Print the result to standard output
    fprintf(stdout, "%lld\n", sum);

    return 0; // success
}
