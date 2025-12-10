/*
 * Program Description
 * -------------------
 * What this program does:
 *   It computes how much radiation (in sieverts, Sv) each person receives after a nuclear blast.
 *   For each person, you give their grid coordinates and a short name. The program prints that
 *   person's dose based on how far they are from the blast.
 *
 * How it works, in plain steps:
 *   1) You run the program with 4 command-line arguments:
 *        - initialBlast   (a floating-point number, like 120.0)
 *        - attenuation    (a floating-point number between 0 and 1, like 0.9)
 *        - blastX         (integer x coordinate of the blast)
 *        - blastY         (integer y coordinate of the blast)
 *
 *   2) The program then reads many lines from standard input (stdin). Each line has:
 *        pointX pointY name
 *      where pointX and pointY are integers and name is a short label (up to 32 chars).
 *
 *   3) For each person at (pointX, pointY), it computes the distance to the blast using
 *      the Pythagorean formula: distance = sqrt( (x1-x2)^2 + (y1-y2)^2 ).
 *
 *   4) It applies an attenuation model:
 *        dose Sv = initialBlast * pow(attentuation, distance)
 *      If attenuation is less than 1, the dose goes down as distance increases.
 *
 *   5) It prints one line per person with their name and the computed dose.
 *
 * Notes on key functions used:
 *   - struct Point: a small custom type to store x and y together.
 *   - distance(): helper function that returns the double precision distance between two Points.
 *   - sqrt and pow (from <math.h>): standard math functions for square root and exponent.
 *   - sscanf / scanf: sscanf reads and converts strings from command line arguments; scanf reads
 *     and converts input lines from stdin while the program runs.
 *   - printf formatting: "%-8s: %8.3lf Sv" prints a left-justified 8-char name, a colon,
 *     then an 8-wide number with 3 digits after the decimal, followed by " Sv".
 *
 * Return codes (why the program might exit with a number):
 *   - 1: Wrong number of command-line arguments were given.
 *   - 2..5: One of the 4 arguments could not be parsed into the required type.
 *           2 = initialBlast failed, 3 = attenuation failed, 4 = blastX failed, 5 = blastY failed.
 */

#include <stdio.h>  // for printf, scanf, sscanf, perror
#include <math.h>   // for sqrt, pow

// A simple type that groups an x and y coordinate together.
struct Point {
    int x;
    int y;
};

// Compute Euclidean distance between two points using the Pythagorean theorem.
// distance = sqrt( (x1 - x2)^2 + (y1 - y2)^2 )
double distance(const struct Point *p1, const struct Point *p2) {
    double dx = (double)p1->x - (double)p2->x;  // horizontal difference
    double dy = (double)p1->y - (double)p2->y;  // vertical difference
    return sqrt(dx*dx + dy*dy);                 // hypotenuse length
}
// Above is Pythagorean to find distance for the final equation.

int main(int argc, char *argv[])
{
    // Expect exactly 4 user-supplied arguments (plus the program name makes argc == 5).
    if (argc != 5)
        return 1;  // Not enough or too many arguments, exit with code 1.

    double initalBlast, radiationDown; // initial blast dose and attenuation factor
    int blastX, blastY;                // blast location on the grid

    // Parse the 4 arguments:
    // argv[1]: initialBlast (double)
    if (sscanf(argv[1], "%lf", &initalBlast) != 1)
        return 2; // could not parse initialBlast

    // argv[2]: attenuation factor (double), often a number between 0 and 1
    if (sscanf(argv[2], "%lf", &radiationDown) != 1)
        return 3; // could not parse attenuation

    // argv[3]: blastX (int)
    if (sscanf(argv[3], "%d", &blastX) != 1)
        return 4; // could not parse blastX

    // argv[4]: blastY (int)
    if (sscanf(argv[4], "%d", &blastY) != 1)
        return 5; // could not parse blastY

    // 1. initial blast  2. attenuation  3. X  4. Y
    struct Point blast = {blastX, blastY}; // store blast location together as a Point

    int pointX, pointY;        // a person's coordinates
    char name[32];             // a short label for the person, up to 31 chars plus null terminator

    // Read many people from stdin until input ends:
    // Each line should look like:  <x> <y> <name>
    // Example:  10 15 Alice
    //
    // scanf returns 3 when it successfully reads all three items.
    while (scanf("%d %d %32s", &pointX, &pointY, name) == 3){
        struct Point person = {pointX, pointY};

        // Radiation model (attenuation with distance):
        // Sv = initialBlast * (attenuation ^ distance)
        //
        // If attenuation is less than 1, then as distance grows, attenuation^distance shrinks,
        // so the final dose drops off with distance.
        double Sv = initalBlast * pow(radiationDown, distance(&person, &blast));

        // Print the name left-justified in a field of width 8: "%-8s"
        // Then a colon and space, then the dose in a field width of 8 with 3 decimals: "%8.3lf"
        // Finally the unit " Sv".
        //
        // Example formatting:
        // Alice   :   12.345 Sv
        printf("%-8s: %8.3lf Sv\n", name, Sv);
    }

    return 0; // normal successful exit
}
