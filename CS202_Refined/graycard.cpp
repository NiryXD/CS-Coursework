/* Program Name: Graycard
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: A program that takes an image and presents it's color information */

#include <iostream>
#include <cstdlib>
#include <cstdio>

void error(const char *message)
{
    fprintf(stderr, "%s\n", message);
    exit(1);
}
// Above is just a basic command to output an error message

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        error("usage: graycard rows cols");
    }
// Above checks if the program is provided a 2 command line argument
    int rows = atoi(argv[1]);
    int cols = atoi(argv[2]);
    // This right here converts the command-line arugments form string to integers 

    if (rows <= 0 || cols <= 0)
    {
        error("usage: graycard rows cols");
    }
    // This checks if they're positive integers

    if (rows % 3 != 0)
    {
        error("rows must be a multiple of three");
    }
    // Right here it checks if it's a multiple of three

    printf("P2\n");
    printf("%d %d\n", cols, rows);
    printf("255\n");
    // This provides the information for the image like indicating it's a plain text file
    // or showing the dimensions of the image

    int section_rows = rows / 3;
    // Dividing the rows into three sections

    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            if (i < section_rows)
            {
                printf("0 ");
            }
            else if (i < 2 * section_rows)
            {
                printf("209 ");
            }
            else
            {
                printf("255 ");
            }
        }
        printf("\n");
    }
    return 0;
    // Looping the seperate colors in order to show the user
}