#include "bitmatrix.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <algorithm>

using namespace std;

/* Bitmatrix class implementation */

Bitmatrix::Bitmatrix(int rows, int cols) {
    if (rows <= 0) throw string("Bad rows");
    if (cols <= 0) throw string("Bad cols");
    M.resize(rows, string(cols, '0'));
}

Bitmatrix::Bitmatrix(const string &fn) {
    ifstream file(fn);
    if (!file.is_open()) throw string("Can't open file");

    string line;
    int cols = -1;
    while (getline(file, line)) {
        string row_data;
        for (char ch : line) {
            if (ch == '0' || ch == '1') {
                row_data += ch;
            } // Ignore whitespace
        }

        if (!row_data.empty()) {
          if (cols == -1) {
             cols = row_data.length();
          } else if (cols != (int)row_data.length()) {
            throw string("Bad file format"); // Inconsistent column count
          }
            M.push_back(row_data);
        }
    }

    if (M.empty() || cols == -1) { // Handle empty file or files with only whitespace
        throw string("Bad file format");
    }

    file.close();
}

Bitmatrix* Bitmatrix::Copy() const {
    Bitmatrix* copy = new Bitmatrix(Rows(), Cols());
    copy->M = M;
    return copy;
}

bool Bitmatrix::Write(const string &fn) const {
    ofstream file(fn);
    if (!file.is_open()) return false;

    for (const string &row : M) {
        file << row << endl;
    }
    
    file.close();
    return true;
}

void Bitmatrix::Print(size_t w) const {
    for (size_t i = 0; i < M.size(); i++) {
        for (size_t j = 0; j < M[i].size(); j++) {
            cout << M[i][j];
            if (w > 0 && (j + 1) % w == 0 && j + 1 != M[i].size()) cout << ' ';
        }
        cout << endl;
        if (w > 0 && (i + 1) % w == 0 && i + 1 != M.size()) cout << endl;
    }
}


bool Bitmatrix::PGM(const string &fn, int p, int border) const {
    ofstream file(fn);
    if (!file.is_open()) return false;

    int rows = M.size(), cols = M[0].size();
    int width = cols * p + (cols + 1) * border;
    int height = rows * p + (rows + 1) * border;

    file << "P2\n" << width << " " << height << "\n255\n"; // Changed to 255 for white

    for (int r = 0; r < rows; ++r) {
        for (int br = 0; br < border; ++br) {
            file << string(width, '0') << '\n'; // Black border
        }
        for (int pr = 0; pr < p; ++pr) {
            for (int c = 0; c < cols; ++c) {
                file << string(border, '0'); // Black border
                string pixel = (M[r][c] == '0') ? "255 " : "100 "; // White or gray
                file << string(p, pixel[0]);       // Fill the square with the color
            }
            file << string(border, '0') << '\n';  // Black border
        }
    }
    for (int br = 0; br < border; ++br) {
        file << string(width, '0') << '\n'; // Black border
    }

    file.close();
    return true;
}


int Bitmatrix::Rows() const {
    return M.size();
}

int Bitmatrix::Cols() const {
    return M.empty() ? 0 : M[0].size();
}

char Bitmatrix::Val(int row, int col) const {
    if (row < 0 || row >= Rows() || col < 0 || col >= Cols()) return 'x';
    return M[row][col];
}

bool Bitmatrix::Set(int row, int col, char val) {
    if (row < 0 || row >= Rows() || col < 0 || col >= Cols()) return false;
    if (val == '0' || val == '1') {
        M[row][col] = val;
        return true;
    }
    return false;
}

bool Bitmatrix::Swap_Rows(int r1, int r2) {
    if (r1 < 0 || r1 >= Rows() || r2 < 0 || r2 >= Rows()) return false;
    swap(M[r1], M[r2]);
    return true;
}

bool Bitmatrix::R1_Plus_Equals_R2(int r1, int r2) {
    if (r1 < 0 || r1 >= Rows() || r2 < 0 || r2 >= Rows()) return false;
    for (size_t i = 0; i < M[r1].size(); ++i) {
        M[r1][i] = ((M[r1][i] - '0') ^ (M[r2][i] - '0')) + '0';
    }
    return true;
}

/* Functions for Bitmatrix operations */

Bitmatrix* Sum(const Bitmatrix *a1, const Bitmatrix *a2) {
    if (a1->Rows() != a2->Rows() || a1->Cols() != a2->Cols()) return nullptr;
    Bitmatrix *result = new Bitmatrix(a1->Rows(), a1->Cols());
    for (int i = 0; i < a1->Rows(); ++i) {
        for (int j = 0; j < a1->Cols(); ++j) {
            result->Set(i, j, ((a1->Val(i, j) - '0') ^ (a2->Val(i, j) - '0')) + '0');
        }
    }
    return result;
}

Bitmatrix* Product(const Bitmatrix *a1, const Bitmatrix *a2) {
    if (a1->Cols() != a2->Rows()) return nullptr;
    Bitmatrix *result = new Bitmatrix(a1->Rows(), a2->Cols());
    for (int i = 0; i < a1->Rows(); ++i) {
        for (int j = 0; j < a2->Cols(); ++j) {
            char sum = '0';
            for (int k = 0; k < a1->Cols(); ++k) {
                sum = ((sum - '0') ^ ((a1->Val(i, k) - '0') & (a2->Val(k, j) - '0'))) + '0';
            }
            result->Set(i, j, sum);
        }
    }
    return result;
}

Bitmatrix* Sub_Matrix(const Bitmatrix *a1, const vector<int> &rows) {
    if (!a1) return nullptr; // Check for null input

    Bitmatrix *result = new Bitmatrix(rows.size(), a1->Cols());
    for (size_t i = 0; i < rows.size(); ++i) {
        int row = rows[i];
        if (row < 0 || row >= a1->Rows()) {
            delete result;
            return nullptr;
        }

        for (int j = 0; j < a1->Cols(); ++j) {
            result->Set(i, j, a1->Val(row, j)); // Use public access methods
        }
    }
    return result;
}


Bitmatrix* Inverse(const Bitmatrix *m) {
    if (!m || m->Rows() != m->Cols()) return nullptr; // Not square

    int n = m->Rows();
    Bitmatrix* result = m->Copy();  // Create a copy to modify
    Bitmatrix* identity = new Bitmatrix(n, n);

    // Create identity matrix
    for (int i = 0; i < n; ++i) {
        identity->Set(i, i, '1');
    }

    // Gaussian elimination
    for (int i = 0; i < n; ++i) {
        // Find pivot
        int pivot = i;
        while (pivot < n && result->Val(pivot, i) == '0') {
            ++pivot;
        }

        if (pivot == n) {
            delete result;
            delete identity;
            return nullptr; // Matrix not invertible (no pivot found)
        }

        if (pivot != i) {
            result->Swap_Rows(i, pivot);
            identity->Swap_Rows(i, pivot);
        }

        // Eliminate other rows
        for (int j = 0; j < n; ++j) {
            if (j != i && result->Val(j, i) == '1') {
                result->R1_Plus_Equals_R2(j, i);
                identity->R1_Plus_Equals_R2(j, i);
            }
        }
    }

    delete result; // We don't need this copy anymore
    return identity;
}


/* BM_Hash class implementation */

BM_Hash::BM_Hash(int size) {
    if (size <= 0) throw string("Bad size");
    Table.resize(size);
}

bool BM_Hash::Store(const string &key, Bitmatrix *bm) {
    int index = hash<string>{}(key) % Table.size();
    for (const auto &entry : Table[index]) {
        if (entry.key == key) return false; // Key already exists
    }
    HTE new_entry = {key, bm};
    Table[index].push_back(new_entry);
    return true;
}

Bitmatrix* BM_Hash::Recall(const string &key) const {
    int index = hash<string>{}(key) % Table.size();
    for (const auto &entry : Table[index]) {
        if (entry.key == key) return entry.bm;
    }
    return nullptr;
}

vector<HTE> BM_Hash::All() const {
    vector<HTE> all_entries;
    for (const auto &bucket : Table) {
        all_entries.insert(all_entries.end(), bucket.begin(), bucket.end());
    }
    return all_entries;
}