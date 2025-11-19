#include <cmath>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
using namespace std;

const size_t ALPHABET_SIZE = 26;

class VigenereCipher {
private:
    string mKey;
    char mVTable[ALPHABET_SIZE][ALPHABET_SIZE];
    void stringToFile(const string message, const string outputFile) const;

public:
    VigenereCipher(const string key);
    void encrypt(const string inputFile, const string outputFile) const;
    void decrypt(const string inputFile, const string outputFile) const;
    void printVTable() const;
    string fileToString(const string inputFile) const; // Making fileToString public
    string makeKeystream(const string message) const;
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

void VigenereCipher::stringToFile(const string message, const string outputFile) const {
    ofstream fout(outputFile);
    if (!fout.is_open()) {
        cerr << "output file " << outputFile << " unable to open." << endl;
        exit(1);
    }
    fout << message;
    fout.close();
}

string VigenereCipher::makeKeystream(const string message) const {
    string keystream;
    size_t keyIndex = 0;
    for (char c : message) {
        if (isalpha(c)) {
            keystream += tolower(mKey[keyIndex % mKey.length()]);
            ++keyIndex;
        } else {
            keystream += c;
        }
    }
    return keystream;
}

VigenereCipher::VigenereCipher(const string key) : mKey(key) {
    for (size_t i = 0; i < ALPHABET_SIZE; ++i) {
        for (size_t j = 0; j < ALPHABET_SIZE; ++j) {
            mVTable[i][j] = 'a' + (i + j) % ALPHABET_SIZE;
        }
    }
}

void VigenereCipher::encrypt(const string inputFile, const string outputFile) const {
    string message = fileToString(inputFile);
    string keystream = makeKeystream(message);
    string encryptedMessage;

    // Perform encryption
    for (size_t i = 0, keyIndex = 0; i < message.length(); ++i) {
    if (isalpha(message[i])) {
        char encryptedChar = mVTable[tolower(mKey[keyIndex % mKey.length()]) - 'a'][tolower(message[i]) - 'a'];
        encryptedMessage += encryptedChar;
        ++keyIndex;
    } else {
        encryptedMessage += message[i];
    }
}

    stringToFile(encryptedMessage, outputFile);
}

void VigenereCipher::decrypt(const string inputFile, const string outputFile) const {
    string encryptedMessage = fileToString(inputFile);
    string keystream = makeKeystream(encryptedMessage);
    string decryptedMessage;

    // Perform decryption
    for (size_t i = 0, keyIndex = 0; i < encryptedMessage.length(); ++i) {
        if (isalpha(encryptedMessage[i])) {
            char keyChar = tolower(mKey[keyIndex % mKey.length()]);
            char encryptedChar = tolower(encryptedMessage[i]);
            char decryptedChar = 'a' + ((encryptedChar - keyChar + 26) % 26);
            decryptedMessage += decryptedChar;
            ++keyIndex;
        } else {
            decryptedMessage += encryptedMessage[i];
        }
    }

    stringToFile(decryptedMessage, outputFile);
}




void VigenereCipher::printVTable() const {
    for (size_t i = 0; i < ALPHABET_SIZE; i++) {
        for (size_t j = 0; j < ALPHABET_SIZE; j++) {
            cout << " " << mVTable[i][j] << " ";
        }
        cout << endl;
    }
}

int main(int argc, char **argv) {
    if (argc != 5) {
        cerr << "usage: ./vigenere inputFile outputFile key [e/d]" << endl;
        return 1;
    }

    string inputFile = argv[1];
    string outputFile = argv[2];
    string key = argv[3];
    char mode = argv[4][0];

    VigenereCipher cipher(key);

    if (mode == 'e') {
        cipher.encrypt(inputFile, outputFile);
        cout << "Keystream: " << cipher.makeKeystream(cipher.fileToString(inputFile)) << endl;
    } else if (mode == 'd') {
        cipher.decrypt(inputFile, outputFile);
        cout << "Keystream: " << cipher.makeKeystream(cipher.fileToString(inputFile)) << endl;
    } else {
        cerr << "Invalid mode. Please specify 'e' for encryption or 'd' for decryption." << endl;
        return 1;
    }

    return 0;
}
