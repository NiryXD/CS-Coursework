#include "pgm.hpp"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
using namespace std;

bool Pgm::Read(const std::string &file) {
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


// TODO Everything past here
bool Pgm::Write(const std::string &file) const {
    ofstream fout(file.c_str());
    if (!fout) {
        cerr << "Error opening file for writing: " << file << endl;
        return false;
    }
    if (Pixels.size() == 0) {
        return false;
    }
    size_t numRows = Pixels.size();
    size_t numCols = Pixels[0].size();

    fout << "P2" << endl;
    fout << numCols << " " << numRows << endl;
    fout << "255" << endl;

    int count = 1; // TODO
    int total = 0; // TODO
    for (size_t r = 0; r < numRows; ++r) {
        for (size_t c = 0; c < numCols; ++c) {
            count++;
            total++;
            if (total < (numRows * numCols)) {
                if ((count != 21)) {
                    fout << Pixels[r][c] << " ";
                }
                else if (count == 21) {
                    fout << Pixels[r][c] << endl;
                    count = 1;
                }
            }
            else {
                fout << Pixels[r][c] << endl;
            }
        }
    }
    fout.close();
    return true;
}

bool Pgm::Create(size_t r, size_t c, size_t pv) {
    if (pv > 255) {
        cerr << "Pixel value must be between 0 and 255." << endl;
        return false;
    }
    Pixels.assign(r, vector<int>(c, pv));
    return true;
}

bool Pgm::Clockwise() {
    size_t numRows = Pixels.size();
    if (numRows == 0) {
        return false;
    }
    size_t numCols = Pixels[0].size();

    vector<vector<int>> rotated(numCols, vector<int>(numRows));

    for (size_t r = 0; r < numRows; ++r) {
        for (size_t c = 0; c < numCols; ++c) {
            rotated[c][numRows - 1 - r] = Pixels[r][c];
        }
    }

    Pixels.swap(rotated);
    return true;
}

bool Pgm::Cclockwise() {
    size_t numRows = Pixels.size();
    if (numRows == 0) {
        return false;
    }
    size_t numCols = Pixels[0].size();

    vector<vector<int>> rotated(numCols, vector<int>(numRows));

    for (size_t r = 0; r < numRows; ++r) {
        for (size_t c = 0; c < numCols; ++c) {
            rotated[numCols - 1 - c][r] = Pixels[r][c];
        }
    }

    Pixels.swap(rotated);
    return true;
}

bool Pgm::Pad(size_t w, size_t pv) {
    if (pv > 255) {
        return false;
    }

    size_t numRows = Pixels.size();
    if (numRows == 0) {
        return false;
    }
    size_t numCols = Pixels[0].size();
    size_t newRows = numRows + 2 * w;
    size_t newCols = numCols + 2 * w;

    vector<vector<int>> padded(newRows, vector<int>(newCols, pv));

    // Copy the original image into the center
    for (size_t r = 0; r < numRows; ++r) {
        for (size_t c = 0; c < numCols; ++c) {
            padded[r + w][c + w] = Pixels[r][c];
        }
    }

    Pixels.swap(padded);
    return true;
}

bool Pgm::Panel(size_t r, size_t c) {
    size_t numRows = Pixels.size();
    if (numRows == 0) return false;
    size_t numCols = Pixels[0].size();

    vector<vector<int>> panel(numRows * r, vector<int>(numCols * c));

    for (size_t i = 0; i < r; ++i) {
        for (size_t j = 0; j < c; ++j) {
            for (size_t a = 0; a < numRows; ++a) {
                for (size_t b = 0; b < numCols; ++b) {
                    panel[i * numRows + a][j * numCols + b] = Pixels[a][b];
                }
            }
        }
    }

    Pixels.swap(panel);
    return true;
}

bool Pgm::Crop(size_t r, size_t c, size_t rows, size_t cols) {
    if (r + rows > Pixels.size() || c + cols > Pixels[0].size()) return false;

    vector<vector<int>> cropped(rows, vector<int>(cols));

    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            cropped[i][j] = Pixels[r + i][c + j];
        }
    }

    Pixels.swap(cropped);
    return true;
}