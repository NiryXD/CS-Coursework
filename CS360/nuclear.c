/* 
 * Program Name: Nuclear Blast Lab 
 * Student Name: Ar-Raniry Ar-Rasyid  
 * Net ID: jzr266  
 * Student ID: 000-663-921  
 * Program Description: Calculate radiation a person recives
 */

#include <stdio.h>
#include <math.h>

struct Point {
    int x;
    int y;
};
double distance(const struct Point *p1, const struct Point *p2) {
    double dx = (double)p1->x - (double)p2->x;
    double dy = (double)p1->y - (double)p2->y;
    return sqrt(dx*dx + dy*dy);
}
// Above is pythagorean to find distance for final equation


int main(int argc, char *argv[])
{
    if (argc != 5)
        return 1;
    // Return if wrong number or arguments

    double initalBlast, radiationDown;
    int blastX, blastY;

    if (sscanf(argv[1], "%lf", &initalBlast) != 1)
        return 2;
    if (sscanf(argv[2], "%lf", &radiationDown) != 1)
        return 3;
    if (sscanf(argv[3], "%d", &blastX) != 1)
        return 4;
    if (sscanf(argv[4], "%d", &blastY) != 1)
        return 5;
    // 1. inital blast 2. attenuations 3. X 4. Y
    
    struct Point blast = {blastX, blastY};
    int pointX, pointY;
    char name[32];

    while (scanf("%d %d %32s", &pointX, &pointY,name ) == 3){
        struct Point person = {pointX, pointY};
        // Equation from write up
        double Sv = initalBlast * pow(radiationDown, distance(&person, &blast));
        // Write out 8 characters wide
        // Embarrassingly I forgot how to left justify so I did ask Chat, I just have to disclose that
        printf("%-8s: %8.3lf Sv\n", name, Sv);
    }

    return 0;
}
