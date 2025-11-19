/* Program Name: Bitmatrix
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: Binary matrix class for creating, manipulating, and storing matrices, and includes hash-based storage */

#include "bitmatrix.hpp"
#include <iostream>
#include <fstream>

using namespace std;

// The write up asks me to use djb_hash(), so below is my implementation of it
// The function iterates over each character, using bit shifts and addition to compute a hash value
unsigned int djb_hash(const string &str) {
    unsigned int hash = 5381;
    for (char c : str) { // I usually use ':' in ternary operators, but in this instance it goes through the aforementioned string
        hash = ((hash << 5) + hash) + c;
    }
    return hash; // Return the hash value
}

// Constructor to create a Bitmatrix with given rows and columns
Bitmatrix::Bitmatrix(int rows, int cols) {
    if (rows <= 0) throw string("Bad rows"); // Throw error if not positive
    if (cols <= 0) throw string("Bad cols"); // Ditto
    M.resize(rows, string(cols, '0')); // Adjust to the given parameters
}

// Reads the matrix from a given filename, each row should have consistent columns
Bitmatrix::Bitmatrix(const string &fn) {
    ifstream file(fn); // Open the file
    if (!file.is_open()) throw string("Can't open file"); // Throw an error if it doesn't open

    string line;
    int cols = -1;
    while (getline(file, line)) { // Read it line by line
        string row_data;
        for (char ch : line) {
            if (ch == '0' || ch == '1') { // Take in ones and zeros
                row_data += ch; 
            }
        }

        if (!row_data.empty()) { // If stuff in row then...
            if (cols == -1) {
                cols = row_data.length(); // This sets the status quo for the following rows
            } else if (cols != (int)row_data.length()) {
                throw string("Bad file format"); // It it aint like the first one it throws it
            }
            M.push_back(row_data); // Add the valid row to the matrix
        }
    }

    if (M.empty() || cols == -1) { // If there's no valid rows or column count wasn't set
        throw string("Bad file format"); // THROW IT
    }

    file.close(); // Close it once errything is done
}

// Returns a new Bitmatrix instance with identical contents
Bitmatrix* Bitmatrix::Copy() const {
    Bitmatrix* copy = new Bitmatrix(Rows(), Cols()); // Copy and paste basically
    copy->M = M; // Copy the current to the new one
    return copy; 
}

// Writes the contents of the matrix to another file
bool Bitmatrix::Write(const string &fn) const {
    ofstream file(fn);
    if (!file.is_open()) return false; // false if can't open

    for (const string &row : M) { // iterate thru it
        file << row << endl;
    }
    
    file.close();
    return true; // Everything works, YIPPEEE
}

// Prints the matrix to the user
void Bitmatrix::Print(size_t w) const {
    for (size_t i = 0; i < M.size(); i++) { // for loop to go thru row
        for (size_t j = 0; j < M[i].size(); j++) { // for loop to go thru columns
            cout << M[i][j]; // Print the current element
            if (w > 0 && (j + 1) % w == 0 && j + 1 != M[i].size()) cout << ' '; // Add space every column, cept at the end of the row
        }
        cout << endl; // New line insertion
        if (w > 0 && (i + 1) % w == 0 && i + 1 != M.size()) cout << endl; // Kinda same concept with New line
    }
}

// Creates a PGM format for the bitmatrix
bool Bitmatrix::PGM(const string &fn, int p, int border) const {
    ofstream file(fn);
    if (!file.is_open()) return false; // False if it can't open

    int rows = M.size(), cols = M[0].size(); // get size of matrix
    int width = cols * p + (cols + 1) * border; // Calculate the PMG width
    int height = rows * p + (rows + 1) * border; // Calculate the PGM height

    file << "P2\n" << width << " " << height << "\n255\n";

    for (int r = 0; r < rows; ++r) { // for loop to go thru the rows
        for (int br = 0; br < border; ++br) {
            for (int c = 0; c < width; ++c) {
                file << "0 ";
            }
            file << '\n';
        }
        for (int pr = 0; pr < p; ++pr) { // Write rows
            for (int c = 0; c < cols; ++c) { // Iterate over each column
                for (int b = 0; b < border; ++b) {
                    file << "0 ";
                }
                for (int pix = 0; pix < p; ++pix) {
                    file << ((M[r][c] == '0') ? "255 " : "100 "); // Ternary operator to do white if zero, else gray
                }
            }
            for (int b = 0; b < border; ++b) {
                file << "0 "; 
            }
            file << '\n';
        }
    }
    for (int br = 0; br < border; ++br) { // Write the bottom border rows
        for (int c = 0; c < width; ++c) {
            file << "0 ";
        }
        file << '\n';
    }

    file.close();
    return true; // Everything works, YIPPEEE
}

// Get the number of rows
int Bitmatrix::Rows() const {
    return M.size(); // Get number of rows
}

// Get the number of columns
int Bitmatrix::Cols() const {
    return M.empty() ? 0 : M[0].size(); // Ternay operator to show if empty or show the actual column
}

char Bitmatrix::Val(int row, int col) const {
    if (row < 0 || row >= Rows() || col < 0 || col >= Cols()) return 'x'; // Check if indices are in bounds
    return M[row][col]; // Give back the position
}

// Set the value of da matrix
bool Bitmatrix::Set(int row, int col, char val) {
    if (row < 0 || row >= Rows() || col < 0 || col >= Cols()) return false; // Check if indices are in bounds
    if (val == '0' || val == '1') { // Check if its a one or a zero
        M[row][col] = val;
        return true; 
    }
    return false; // Show false if the previous 'it' statements didn't catch it
}

// Swaps the contents of two rows
bool Bitmatrix::Swap_Rows(int r1, int r2) {
    if (r1 < 0 || r1 >= Rows() || r2 < 0 || r2 >= Rows()) return false; // Check if row indices are in bounds
    swap(M[r1], M[r2]); // Swap the two rows
    return true;
}

// Performs row1 = row1 XOR row2
bool Bitmatrix::R1_Plus_Equals_R2(int r1, int r2) {
    if (r1 < 0 || r1 >= Rows() || r2 < 0 || r2 >= Rows()) return false; // Check if row indices are in bounds
    for (size_t i = 0; i < M[r1].size(); i++) { // for loop to go through columns
        M[r1][i] = ((M[r1][i] - '0') ^ (M[r2][i] - '0')) + '0'; // Perform XOR
    }
    return true; 
}

// Sum of two Bitmatrices using XOR
Bitmatrix* Sum(const Bitmatrix *a1, const Bitmatrix *a2) {
    if (a1->Rows() != a2->Rows() || a1->Cols() != a2->Cols()) return nullptr; // Return null if dimensions don't match
    Bitmatrix *result = new Bitmatrix(a1->Rows(), a1->Cols()); // Make new matrix
    for (int i = 0; i < a1->Rows(); i++) { // Iterate each row
        for (int j = 0; j < a1->Cols(); ++j) { // Iterate each column
            result->Set(i, j, ((a1->Val(i, j) - '0') ^ (a2->Val(i, j) - '0')) + '0'); // Set result element as XOR of inputs
        }
    }
    return result;
}

// Multiplies two matrices using AND and XOR operations, returns a new Bitmatrix with the result
Bitmatrix* Product(const Bitmatrix *a1, const Bitmatrix *a2) {
    if (a1->Cols() != a2->Rows()) return nullptr; // Return null if matrices can't be multiplied
    Bitmatrix *result = new Bitmatrix(a1->Rows(), a2->Cols()); // Create a new matrix
    for (int i = 0; i < a1->Rows(); i++) { // Go over each row of the first matrix
        for (int j = 0; j < a2->Cols(); ++j) { // Go over each column of the second matrix
            char sum = '0'; // Initialize sum
            for (int k = 0; k < a1->Cols(); ++k) { // Iterate over elements
                sum = ((sum - '0') ^ ((a1->Val(i, k) - '0') & (a2->Val(k, j) - '0'))) + '0'; // Perform AND and XOR, accumulate in sum
            }
            result->Set(i, j, sum);
        }
    }
    return result;
}

// Extracts specified rows from the input matrix and returns them as a new Bitmatrix
Bitmatrix* Sub_Matrix(const Bitmatrix *a1, const vector<int> &rows) {
    if (!a1) return nullptr; // Check if the input matrix is good

    Bitmatrix *result = new Bitmatrix(rows.size(), a1->Cols()); // Create a new matrix
    for (size_t i = 0; i < rows.size(); i++) { // Iterate over the list of row indices
        int row = rows[i];
        if (row < 0 || row >= a1->Rows()) { // Check if row index is good
            delete result; // Delete the result matrix if there's an bad row
            return nullptr; // Return null to if failed
        }

        for (int j = 0; j < a1->Cols(); ++j) { // Iterate each column
            result->Set(i, j, a1->Val(row, j)); // Set the value in the sub-matrix
        }
    }
    return result;
}


// Gaussian elimination to find the inverse of a square matrix, returns null if not possible
Bitmatrix* Inverse(const Bitmatrix *m) {
    if (!m || m->Rows() != m->Cols()) return nullptr; // Return null if the matrix isn't square

    int n = m->Rows(); // Get dimension of the matrix
    Bitmatrix* result = m->Copy(); // Create a copy of the matrix
    Bitmatrix* identity = new Bitmatrix(n, n); // Create an identity matrix of the same size

    for (int i = 0; i < n; i++) {
        identity->Set(i, i, '1'); // Set diagonal elements to one
    }

    // Gaussian elimination
    for (int i = 0; i < n; i++) {
        int pivot = i;
        while (pivot < n && result->Val(pivot, i) == '0') {
            ++pivot; // Find a row with pivot
        }

        if (pivot == n) { // If no pivot is found, just trash everything
            delete result; 
            delete identity; 
            return nullptr; 
        }

        if (pivot != i) { // If pivot is not in the current row
            result->Swap_Rows(i, pivot); // Swap the current row with the pivot row
            identity->Swap_Rows(i, pivot);
        }

        // Eliminate all other elements
        for (int j = 0; j < n; ++j) {
            if (j != i && result->Val(j, i) == '1') { // If the element is non-zero...
                result->R1_Plus_Equals_R2(j, i); // Subtract the pivot row from the current row
                identity->R1_Plus_Equals_R2(j, i); // Do it again
            }
        }
    }

    delete result;
    return identity;
}

// Initializes a hash table with the given number
BM_Hash::BM_Hash(int size) {
    if (size <= 0) throw string("Bad size"); // GET IT OUT, if it's not positive
    Table.resize(size);
}

// Uses a hash function to determine the index and stores the matrix 
bool BM_Hash::Store(const string &key, Bitmatrix *bm) {
    int index = djb_hash(key) % Table.size(); // Calculate the hash index
    for (const auto &entry : Table[index]) { // Check for existing entries
        if (entry.key == key) return false; // Return false if key already there
    }
    HTE new_entry = {key, bm}; // Create a new entry
    Table[index].push_back(new_entry); // Add the new entry
    return true;
}

// Searches the hash table for the given key and returns it if found
Bitmatrix* BM_Hash::Recall(const string &key) const {
    int index = djb_hash(key) % Table.size(); // Calculate the hash index
    for (const auto &entry : Table[index]) { // Iterate the entries
        if (entry.key == key) return entry.bm; // Return the Bitmatrix if key is found
    }
    return nullptr;
}

// Returns vector containing all hash table entries
vector<HTE> BM_Hash::All() const {
    vector<HTE> all_entries; // Vector that holds all the entries
    for (const auto &bucket : Table) { // Iterate over buckets in the hash table
        all_entries.insert(all_entries.end(), bucket.begin(), bucket.end()); // Add all entries
    }
    return all_entries;
}