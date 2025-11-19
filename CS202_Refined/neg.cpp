/* Program Name: Neg
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: A program that takes an image and inverts it's colors and presents it to the user */

#include <iostream>
#include <vector>
#include <string>
#include <cstdio>

using namespace std;

struct Image
{
    string particular;
    int width, height, maxval;
    vector<int> pixels;
    // Here is just information about the image
    // Like the width and the height, and the max value
};

void error(const string &message)
{
    cerr << "Error: " << message << endl;
    exit(1);
}
// Just helping me with printing error statements

Image read()
{
    Image img;
    if (!(cin >> img.particular) || img.particular != "P2")
    {
        error("Invalid PGM file - must start with P2");
    }
    // Reading if the format is allowed to be taken

    if (!(cin >> img.width >> img.height) || img.width <= 0 || img.height <= 0)
    {
        error("Invalid PGM dimensions");
    }
    //Checking if the dimensions are able to be taken 

    if (!(cin >> img.maxval) || img.maxval != 255)
    {
        printf("Bad PGM file -- No 255 following the rows and columns");
    }
    // Capping the max grayscale at 255

    img.pixels.resize(img.width * img.height);
    for (int &pixel : img.pixels)
    {
        if (!(cin >> pixel) || pixel < 0 || pixel > 255)
        {
            error("Invalid pixel value");
        }
    }
    // reading the pixel values and validating each one

    int xtra;
    if (cin >> xtra)
    {
        error("Too many pixels in input");
    }
    // check if there's extra pixels

    return img;
    //return the image object with data
}

void write(const Image &img)
{
    printf("P2\n%d %d\n225\n", img.width, img.height);
    for (int i = 0; i < img.pixels.size(); i++)
    {
        printf("%d", 255 - img.pixels[i]);
        if ((i + 1) % img.width == 0 || i == img.pixels.size() - 1)
        {
            printf("\n");
        }
        else
        {
            printf(" ");
        }
    }
}
// First the PGM file header is outputted
// Look through the image and then invert the color value
//print a newline after every last pixel

int main()
{
    try
    {
        Image img = read();
        write(img);
    }
    catch (const exception &e)
    {
        error(e.what());
    }
    return 0;
}
//right here is all the objects put into main
//with try I can try reading the input and writing 
// the new inverted one
// otherwise we catch it with an error check