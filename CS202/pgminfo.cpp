/* Program Name: PGMInfo
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: A program that takes an image and presents it's information */
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <cstdio>

using namespace std;

int main() {
    string particular;
    // Particular is storing the P2 number given from the image
    int rows, cols, maxval;
    // other information given to help us further down the road
    vector<int> pixels;

    if (!(cin >> particular) || particular != "P2") {
        cerr << "Bad PGM file -- first word is not P2\n";
        return 1;
        // error message if the format is wrong and then exits
    }

    if (!(cin >> cols >> rows) || cols <= 0 || rows <= 0) {
        cerr << "Bad PGM file -- No row specification\n";
        return 1;
        // error message if the dimensions are invalid
    }

    if (!(cin >> maxval) || maxval != 255) {
        cerr << "Bad PGM file -- No 255 after rows\n";
        // error message if the max value isn't 255
    }

    int pixel;
    long long sum = 0;
    for (int i = 0; i < rows * cols; i++) {
        if (!(cin >> pixel) || pixel < 0 || pixel > 255) {
            cerr << "Bad PGM file -- pixel " << maxval << " is not a number between 0 and 255\n";
            return 1;
        }
        // Above just ensures that it is beetween 0 and 255
        pixels.push_back(pixel);
        sum += pixel;
        // add the valid pixel value to the vector
        // add the pixel value to the sum for later average calcualtion
    }

    if (cin >> pixel)
    {
        cerr << "Bad PGM file -- Extra stuff after the pixels\n";
        return 1;
    }

    double average = static_cast<double>(sum) / (rows * cols);
    // calculate average pixel sum

    printf("# Rows:    %8d\n", rows);
    printf("# Columns: %8d\n", rows);
    printf("# Pixels:  %8d\n", rows * cols);
    printf("Avg Pixel: %8.3d\n", average);
    //printing all the information to the user

    return 0;
}