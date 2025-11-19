 /* Program Name: Vigenère Cipher
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description: Encoding and Decoding via Vigenere Cipher using predetermined .txt files*/

//  gradescript command: `python3.11 scripts/test.py... `

#include <cmath> // All of the pound includes I know
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <stdlib.h> 
using namespace std;

const size_t ALPHABET_SIZE = 26;

class VigenereCipher {
  private:
    string mKey;
    char mVTable[ALPHABET_SIZE][ALPHABET_SIZE];
     //string fileToString(const string inputFile) const;
    void stringToFile(const string message, const string outputFile) const;
     // string makeKeystream(const string message) const;

  public:
    VigenereCipher(const string key);
    void encrypt(const string inputFile, const string outputFile) const;
    void decrypt(const string inputFile, const string outputFile) const;
    void printVTable() const;
    string fileToString(const string inputFile) const;
    string makeKeystream(const string message) const; // In the template this use to be private, but I moved it since I need Keystream 
    // in my output to the user. 
};

string VigenereCipher::fileToString(const string inputFile) const {
    ifstream fin(inputFile);
    string message;
    char character;
    if (!fin.is_open()) {
        cerr << "file " << inputFile << " unable to open." << endl;
        exit(1);
        }
    while (fin.get(character)) {
        message.push_back(character);
    }
    fin.close();
    return message;
}

// Not much to say here, it's given.

void VigenereCipher::stringToFile(const string message,
                                  const string outputFile) const {
    ofstream fout(outputFile);
    if (!fout.is_open()) {
        cerr << "output file " << outputFile << " unable to open." << endl;
        exit(1);
    }
    fout << message;
    fout.close();
}

// Nothing here either. I just feel like it's empty if I don't comment here.

string VigenereCipher::makeKeystream(const string message) const {
    string stream;
    size_t index = 0;
    for (char a : message) {
        if (isalpha(a)) {
            stream += tolower(mKey[index % mKey.length()]);
            ++index
        } else {
            stream += a
        }
        }
        return stream;
    }

// 'isalpha' is a command where it detects whether a character is within the phonetic alphabet, found it on some internet forum. 
// the basic idea is to confirm whether something is a letter, and then use the 26x26 and 'mKey' to find the correlating letter.

VigenereCipher::VigenereCipher(const string key) {
    for (size_t a = 0; a < ALPHABET_SIZE; ++a) {
        for (size_t b = 0; b < ALPHABET_SIZE; ++b) {
            mVTable[a][b] = 'a' + (a + b) % ALPHABET_SIZE;
        }
    }
}

// This is basically a for loop creating the 26x26 that will later be refrenced to when encrypting and decrypting. It just
// runs through the alphabet.

void VigenereCipher::encrypt(const string inputFile,
                             const string outputFile) const {}

void VigenereCipher::decrypt(const string inputFile,
                             const string outputFile) const {}

void VigenereCipher::printVTable() const {
     for (size_t a = 0; a < ALPHABET_SIZE; a++) {
        for (size_t b = 0; b < ALPHABET_SIZE; b++) {
            cout << " " << mVTable[a][b] << " ";
        }
        cout << endl;
    }
}

// Very similar to VigenereCipher from earlier. This just a print statement in it.

int main(int argc, char **argv) {
 if (argc != 5) {
        cerr << "usage: ./vigenere inputFile outputFile key [e/d]" << endl;
        return 1;
    }

    // This error check was the first case of the python script. I just ripped it from there.

    string inputFile = argv[1];
    string outputFile = argv[2];
    string key = argv[3];
    char mode = argv[4][0];

    // TA helped me on this, never caught his name. I'd give him credit though.
    // It's just like the boiling program, it's checking for inputs.

    VigenereCipher cipher(key);

    if (mode == 'e') {
        cipher.encrypt(inputFile, outputFile);
        cout << "Keystream: " << cipher.makeKeystream(cipher.fileToString(inputFile)) << endl;
    } else if (mode == 'd') {
        cipher.decrypt(inputFile, outputFile);
        cout << "Keystream: " << cipher.makeKeystream(cipher.fileToString(inputFile)) << endl;
    } else {
        cerr << "Choose 'e' or 'd'" << endl;
        return 1;
    }
    
    // 'e' is for encryption. I moved makeKeystream into public to make this possible. I debugged it by looking
    // at my mistake from the python script and edited it to make the next test case. There's no particular
    // error check.

    return 0;
}