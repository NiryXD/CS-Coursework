/* Program Name: PPM Manipulation
Student Name: Ar-Raniry Ar-Rasyid
NED ID: jzr266
Student ID: 000-663-921
Program Description: Read in a picture, and then be able to manipulate it*/

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

struct Pixel {
    unsigned int r;
    unsigned int g;
    unsigned int b;
};

// Learned what 'unsigned' is through CS102 Handbook

class Picture {
  private:
    size_t width;
    size_t height;
    int maxIntensity;
    vector<Pixel> pixels;

  public:
    Picture();

    const Pixel &getPixel(const size_t row, const size_t column) const;
    Pixel &getPixel(const size_t row, const size_t column);
    void setPixel(const size_t row, const size_t column, const Pixel &pixel);

    void invert();
    void flipY();
    void flipX();


    bool readInput(istream &in);
    void writeOutput(ostream &out);
};

const Pixel &Picture:: getPixel (const size_t row, const size_t collum) const {
  size_t indx = row * width + collum;
  return pixels [indx];
}

//  This is done through reading in the vector of the image.

void Picture:: setPixel (const size_t row, const size_t collum, const Pixel &pixel) {
  size_t indx = row * width + collum;
  pixels[indx] = pixel;
}

// How this works is it reads in the consts found in the vector of pixels. 
 

void Picture::invert() {
  for (Pixel &pixels : pixels) {
    pixels.r = maxIntensity - pixels.r;
    pixels.g = maxIntensity - pixels.g;
    pixels.b = maxIntensity - pixels.b;
  }
}

// Created a invert object to be recalled later

void Picture::flipY() {
  vector<Pixel> pixNow (pixels);
  for (size_t collum = 0; collum < width; collum++) {
    for (size_t row = 0; row < height; row++) {
      pixels [row * width + collum] = pixNow [(height - row - 1) * width + collum];
    }
  }
}

// Using a vector we can keep the current image so we can edit it later. Within the nested loop the flipping begins. 
// By iterating each time the pixel data can be rewritten until it's completely fippled to the Y axis
// The same proceduce is done to flip to the X axis

void Picture::flipX() {
  vector<Pixel> pixNow (pixels);
  for (size_t collum = 0; collum < width; collum++) {
    for (size_t row = 0; row < height; row++) {
      pixels [row * width + collum] = pixNow [height * row + (width - collum - 1)];
    }
  }
}




bool Picture::readInput(istream &in) {

    return true;
}

int main(int argc, char **argv) {

// Below are the first few error checks 

if (argc < 3 || argc > 4) {
  cerr << "Usage: ./ppm <input file> <output file> [i|I|x|X|y|Y]" << endl;
  return 1;
}

string input = argv[1];
string output = argv[2];
char manip = '\0';

if (argc == 4) {
  manip = argv[3][0];
  if (manip != 'i' && manip != 'I' && manip != 'x' && manip != 'X' && manip != 'y' && manip != 'Y') {
    cerr << "Unknown operation: " << manip << endl;
    return 1;
  }
}

// When a user enters in an undecipherable char, we will let them know it's none of the avaliable operations.

ifstream fin;
if( input != "-") {
  fin.open(input);
  if (!fin) {
    cerr << "Unable to open " << input << "." << endl;
    return 1;
  }
}

// Both the output and input error messages are structured the same. Also recognizes the the '-' problem.

ifstream fout;
if( output != "-") {
  fout.open(output);
  if (!fout) {
    cerr << "Unable to open " << output << "." << endl;
    return 1;
  }
}
  
  cout << " Successfully manipulated and saved the image." << endl;
  
    return 0;
}