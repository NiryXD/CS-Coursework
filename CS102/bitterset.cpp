#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class BITSET {
    vector<int> mSets;

public:
    BITSET();
    bool Test(int index) const;
    void Set(int index);
    void Clear(int index);
    int GetNumSets() const;
    int GetSet(int index) const;
};

string ToBinary(int bit_set, int spacing = 4);

int main() {
    BITSET sets;
    string command;
    int which;

    do {
        cout << "> ";
        if (!(cin >> command) || "q" == command) {
            break;
        }

        if ("t" == command) {
            if (!(cin >> which)) {
                cout << "Invalid index\n";
            } else {
                cout << sets.Test(which) << '\n';
            }
        } else if ("s" == command) {
            if (!(cin >> which)) {
                cout << "Invalid index\n";
            } else {
                sets.Set(which);
            }
        } else if ("c" == command) {
            if (!(cin >> which)) {
                cout << "Invalid index\n";
            } else {
                sets.Clear(which);
            }
        } else if ("g" == command) {
            int spacing;
            string line;
            getline(cin, line);
            istringstream sin{line};
            if (!(sin >> which)) {
                cout << "Invalid index\n";
            } else {
                if (!(sin >> spacing)) {
                    spacing = 4;
                }
                cout << ToBinary(sets.GetSet(which), spacing) << '\n';
            }
        } else if ("n" == command) {
            cout << sets.GetNumSets() << '\n';
        } else {
            cout << "Unknown command '" << command << "'\n";
        }
    } while (true);

    return 0;
}

BITSET::BITSET() : mSets(1, 0) {}

bool BITSET::Test(int index) const {
    int which_set = index / 32;
    int which_bit = index % 32;
    if (which_set >= static_cast<int>(mSets.size())) {
        return false;
    }
    return mSets[which_set] & (1 << which_bit);
}

void BITSET::Set(int index) {
    int which_set = index / 32;
    int which_bit = index % 32;
    if (which_set >= static_cast<int>(mSets.size())) {
        mSets.resize(which_set + 1, 0);
    }
    mSets[which_set] |= (1 << which_bit);
}

void BITSET::Clear(int index) {
    int which_set = index / 32;
    int which_bit = index % 32;
    if (which_set < static_cast<int>(mSets.size())) {
        mSets[which_set] &= ~(1 << which_bit);
        while (!mSets.empty() && mSets.back() == 0) {
            mSets.pop_back();
        }
    }
}

int BITSET::GetNumSets() const {
    if (mSets.empty()) {
        // If there are no sets, resize to have at least one set
        return 1;
    } else {
        return static_cast<int>(mSets.size());
    }
}

int BITSET::GetSet(int index) const {
    return index < static_cast<int>(mSets.size()) ? mSets[index] : 0;
}

string ToBinary(int bit_set, int spacing) {
    string ret;
    int digits = 32; // Assuming 32-bit integers
    bool firstDigit = true;
    
    for (int i = digits - 1; i >= 0; --i) {
        if (bit_set & (1 << i)) {
            ret += '1';
        } else {
            ret += '0';
        }
        
        if ((32 - i) % spacing == 0 && i != 0) {
            ret += ' ';
        }
    }
    return ret;
}
