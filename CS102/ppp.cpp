#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

struct Pixel {
    unsigned int red;
    unsigned int green;
    unsigned int blue;
};

class Picture {
private:
    int width;
    int height;
    int maxIntensity;
    vector<Pixel> pixels;

public:
    Picture() : width(0), height(0), maxIntensity(0) {}

    const Pixel &getPixel(int row, int column) const {
        return pixels[row * width + column];
    }

    Pixel &getPixel(int row, int column) {
        return pixels[row * width + column];
    }

    void setPixel(int row, int column, const Pixel &pixel) {
        pixels[row * width + column] = pixel;
    }

    void invert() {
        for (auto &pixel : pixels) {
            pixel.red = maxIntensity - pixel.red;
            pixel.green = maxIntensity - pixel.green;
            pixel.blue = maxIntensity - pixel.blue;
        }
    }

    void flipX() {
        for (int row = 0; row < height / 2; ++row) {
            for (int col = 0; col < width; ++col) {
                swap(pixels[row * width + col], pixels[(height - 1 - row) * width + col]);
            }
        }
    }

    void flipY() {
        for (int col = 0; col < width / 2; ++col) {
            for (int row = 0; row < height; ++row) {
                swap(pixels[row * width + col], pixels[row * width + (width - 1 - col)]);
            }
        }
    }

    bool readInput(istream &in) {
        string line;
        getline(in, line);
        if (line != "P3") {
            cerr << "Error: Not a PPM file." << endl;
            return false;
        }
        getline(in, line); // Read dimensions line
        istringstream dimensions(line);
        dimensions >> width >> height;
        getline(in, line); // Read max intensity line
        istringstream intensity(line);
        intensity >> maxIntensity;

        // Reserve space for pixels
        pixels.resize(width * height);

        // Read pixel values
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                Pixel pixel;
                in >> pixel.red >> pixel.green >> pixel.blue;
                if (pixel.red > maxIntensity || pixel.green > maxIntensity || pixel.blue > maxIntensity) {
                    cerr << "Warning: Pixel value exceeds maximum intensity. Setting to max intensity." << endl;
                    pixel.red = pixel.green = pixel.blue = maxIntensity;
                }
                pixels[i * width + j] = pixel;
            }
        }
        return true;
    }

    void writeOutput(ostream &out) {
        out << "P3\n";
        out << width << " " << height << "\n";
        out << maxIntensity << "\n";
        for (const auto &pixel : pixels) {
            out << pixel.red << " " << pixel.green << " " << pixel.blue << "\n";
        }
    }
};
