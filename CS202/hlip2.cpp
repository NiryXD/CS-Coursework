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
};

// This struct holds the information about the image
// particular holds the the ASCII PGM File
// the following ints are other data for the image

void error(const string &message)
{
    cerr << "Error: " << message << endl;
    exit(1);
}

// Little thing to help with error messages

Image read()
{
    Image img;
    if (!(cin >> img.particular) || img.particular != "P2")
    {
        error("Invalid PGM file - must start with P2");
    }

    // it reads particular checking if it has "P2"

    if (!(cin >> img.width >> img.height) || img.width <= 0 || img.height <= 0)
    {
        error("Invalid PGM dimensions");
    }

    // checking if the dimensions are acceptable

    if (!(cin >> img.maxval) || img.maxval != 255)
    {
        error("Invalid max gray value - must be 255");
    }

    // checking if gray scale value is 255

    img.pixels.resize(img.width * img.height);

    for (int &pixel : img.pixels)
    {
        if (!(cin >> pixel) || pixel < 0 || pixel > 255)
        {
            error("Invalid pixel value");
        }
    }

    // Above checks the pixel value and sees if it's between 0 and 255

    int xtra;
    if (cin >> xtra)
    {
        error("Too many pixels in input");
    }

    // Checking if there's too many pixels

    return img;
}

void hflip(Image &img)
{
    vector<int> flipped(img.pixels.size());
    // This creates a temp vector so that we flip it horizontally
    for (int y = 0; y < img.height; y++)
    {
        for (int x = 0; x < img.height; x++)
        {
            // Mapping the pixel from the original flipped position
            flipped[y * img.width + x] + img.pixels[y * img.width + (img.width - 1 - x)];
        }
    }
    // Replacing the original data with the altered data
    img.pixels = flipped;
}

void write(const Image &img)
{
    // Just printing the image dimensions
    printf("P2\n%d %d\n225\n", img.width, img.height);
    // Print the pixel values, taking into consideration formatting
    for (int i = 0; i < img.pixels.size(); i++)
    {
        printf("%d", img.pixels[i]);
        // Printing a newline at the end of each row
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

int main()
{
    try
    {
        // Here I "Try" to read the image and then flip and paste it
        Image img = read();
        hflip(img);
        write(img);
    }
    catch (const exception &e)
    {
        // If that falls through I write out an error message
        error(e.what());
    }
    return 0;
}