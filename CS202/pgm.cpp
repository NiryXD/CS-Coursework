/* Program Name: PGM
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: A program that takes an image and and messes around with it in various ways */
#include "pgm.hpp"
#include <fstream> 
#include <sstream>
#include <iostream> 
#include <iomanip>  

using namespace std;

bool Pgm::Read(const std::string &file)
{
  ifstream fin;
  string s;
  size_t i, j, r, c, v;

  fin.open(file.c_str());
  if (fin.fail()) return false;

  if (!(fin >> s)) return false;
  if (s != "P2") return false;
  if (!(fin >> c) || c <= 0) return false;
  if (!(fin >> r) || r <= 0) return false;
  if (!(fin >> i) || i != 255) return false;

  Pixels.resize(r);
  for (i = 0; i < r; i++) {
    Pixels[i].clear();
    for (j = 0; j < c; j++) {
      if (!(fin >> v) || v > 255) return false;
      Pixels[i].push_back(v);
    }
  }
  if (fin >> s) return false;
  fin.close();
  return true;
}

// Function to write to the user
bool Pgm::Write(const std::string &file) const {
    ofstream output(file.c_str());
    if (!output) {
        cerr << "Error opening file for writing: " << file << endl;
        // if the file can't be open, here's an error check to visualize the problem
        return false;
    }
    if (Pixels.size() == 0) {
        return false;
    }
    
    size_t row = Pixels.size();
    size_t col = Pixels[0].size();

    // Below is just the things needed for the user to understand it's content
    // As well as me passing the test cases
    output << "P2" << endl;
    output << col << " " << row << endl;
    output << "255" << endl;

    int count = 0;
    int all = 0;
    for (size_t r = 0; r < row; r++) {
    
    for (size_t c = 0; c < col; c++) {
            count++;
            all++;
            if (all < (row * col)) {
            if ((count != 20)) {
                    output << Pixels[r][c] << " ";
            }
            else if (count == 20) {
                    output << Pixels[r][c] << endl;
                    count = 0;
            }
            }
            else {
                output << Pixels[r][c] << endl;
            }
        }
    }
    output.close();
    return true;
}
// Above is me inializing a count to see me going through the rows and columns
// While doing the incrementations I print the pixels, checking if it's the 20th or not
// if it is the 20th, we create a new line. Basing it all off the requirements of this lab

bool Pgm::Create(size_t r, size_t c, size_t pv) {
    if (pv > 255) {  
        return false;
    }
    // Above checks for the range and sees if it's viable

    Pixels.assign(r, vector<int>(c, pv));  
    return true; 
}
// Utilize a vector right here

bool Pgm::Clockwise() {
    if (Pixels.empty()) return false;  // If there isn't any data, return false
    
    size_t row = Pixels.size();
    size_t col = Pixels[0].size();
    // Here I'm making an array to utilize for the swapped images
    vector<vector<int> > new_pixels(col, vector<int>(row));  

    for (size_t r = 0; r < row; r++) {
        for (size_t c = 0; c < col; c++) {
            new_pixels[c][row - r - 1] = Pixels[r][c];  
        }
    }
    // Above is me finding the new location for the pixels using for loops

    Pixels = new_pixels;  // updating everything to the rotated image
    return true;  
}

bool Pgm::Cclockwise() {
    if (Pixels.empty()) return false; 

    size_t row = Pixels.size();
    size_t col = Pixels[0].size();
    vector<vector<int> > new_pixels(col, vector<int>(row));  

    for (size_t r = 0; r < row; r++) {
        for (size_t c = 0; c < col; c++) {
            new_pixels[col - c - 1][r] = Pixels[r][c];
        }
    }

    Pixels = new_pixels; 
    return true; 
}
// Above is identical is to clockwise but this time the rotation is differnt. 

bool Pgm::Pad(size_t w, size_t pv) {
    if (pv > 255) {  // Checking if it's within parameters
        return false;
    }

    size_t row = Pixels.size();
    size_t col = Pixels[0].size();
    // Create a new larger image, surronding the original
    vector<vector<int> > new_pixels(row + 2 * w, vector<int>(col + 2 * w, pv));

    // Paste the copy of the image into the center
    for (size_t r = 0; r < row; r++) {
    for (size_t c = 0; c < col; c++) {
            new_pixels[r + w][c + w] = Pixels[r][c];
        }
    }

    Pixels = new_pixels;  // updating everything to the padded image
    return true; 
}

bool Pgm::Panel(size_t r, size_t c) {
    if (Pixels.empty()) return false;  
    // Checking if anything is there.

    size_t orig_row = Pixels.size();
    size_t orig_col = Pixels[0].size();
    // Making space so there's enough for however much duplicates the user wants
    vector<vector<int> > new_pixels(orig_row * r, vector<int>(orig_col * c));

    // Putting the image down serval times over.
    for (size_t i = 0; i < r; i++) {
    for (size_t j = 0; j < c; j++) {
    for (size_t rr = 0; rr < orig_row; rr++) {
    for (size_t cc = 0; cc < orig_col; cc++) {
                    new_pixels[i * orig_row + rr][j * orig_col + cc] = Pixels[rr][cc];
                }
            }
        }
    }

    Pixels = new_pixels;  // updating everything to the duped image
    return true;
}

bool Pgm::Crop(size_t r, size_t c, size_t row, size_t col) {
    // checks to see if cropping goes outta bounds, like too much cropping theres nothing
    if (r + row > Pixels.size() || c + col > Pixels[0].size()) {
        cerr << "Invalid crop dimensions." << endl;
        return false;
    }

    // Create something new to hold the new image
    vector<vector<int> > new_pixels(row, vector<int>(col));

    // Take a chunk of the original image
    for (size_t i = 0; i < row; i++) {
    
    for (size_t j = 0; j < col; j++) {
            new_pixels[i][j] = Pixels[r + i][c + j];
        }
    }

    Pixels = new_pixels;  // updating everything to the cropped image
    return true;
}
