/* Program Name: Bitset
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * TAs that helped: Abram Bradley
 * Program Description: Program meant to store a large number of Boolean Values */

//  - auto-format in vim: gg=G in normal mode, in vscode: alt+shift+f
//  - gradescript command: `python3.11 scripts/test.py bitset.cpp`

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

class BITSET
{
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
int main()
{
    BITSET sets;
    string command;
    int which;
    do
    {
        cout << "> ";
        if (!(cin >> command) || "q" == command)
        {
            // If cin fails for any reason, it usually means
            // an EOF state, break and quit.
            break;
        }
        // These are the commands avalible to the user. 't' prints 1 or 0. 's' sets bits to 1. 'c' does the same but for 0.
        // This is basically the same things of what the Program page says.
        if ("t" == command)
        {
            if (!(cin >> which))
            {
                cout << "Invalid index\n";
            }
            else
            {
                cout << sets.Test(which) << '\n';
            }
        }
        else if ("s" == command)
        {
            if (!(cin >> which))
            {
                cout << "Invalid index\n";
            }
            else
            {
                sets.Set(which);
            }
        }
        else if ("g" == command)
        {
            int spacing;
            string line;
            getline(cin, line);
            istringstream sin{line};
            if (!(sin >> which))
            {
                cout << "Invalid index\n";
            }
            else
            {
                if (!(sin >> spacing))
                {
                    spacing = 4;
                }
                cout << ToBinary(sets.GetSet(which), spacing) << '\n';
            }
        }
        else if ("c" == command)
        {
            cout << sets.GetNumSets() << '\n';
        }
        // The comment here previously asked me to finish it up here, but it seems to need no editing.
        else
        {
            cout << "Unknown command '" << command << "'\n";
        }
    } while (true);
    return 0;
}

string ToBinary(int bit_set, int spacing)
{
    string ret;
    for (int i = 31; i >= 0; i--)
    {
        if (bit_set & (1 << i))
        {
            // Wrote in the operation, it ensures that the code matches binary.
            ret += '1';
        }
        else
        {
            ret += '0';
        }
        // the explanation was given to me by the TAs. Through the announcments on Discord. Thank you very kindly
        if (i % spacing == 0 && i != 0)
        {
            ret += ' ';
        }
    }
    return ret;
}

BITSET::BITSET() : mSets(1, 0) {}

bool BITSET::Test(int index) const
{
    // Recall that each set has 32 bits
    int which_set = index / 32; // Completing the arithmetic using the hint above
    int which_bit = index % 32;
    if (which_set >= static_cast<int>(mSets.size()))
    {
        return false;
    }
    return mSets[which_set] & (1 << which_bit); // Tests the bit and then returns it
}

void BITSET::Set(int index)
{
    int which_set = index / 32; // Similar to what I've done above
    int which_bit = index % 32;
    if (which_set >= mSets.size())
    {
        mSets.resize(which_set + 1, 0);
    }
    // mSets.at(which_set) = /* ... */;
    mSets[which_set] |= (1 << which_bit);
    // I opted to use an OR operator to set bit which_bit
}

void BITSET::Clear(int index)
{
    int which_set = index / 32;
    int which_bit = index % 32;
    if (which_set < GetNumSets())
    {
        mSets.at(which_set) &= ~(1 << which_bit); // Clears the bit
        while (!mSets.empty() && mSets.back() == 0)
        {
            mSets.pop_back(); // truncate empty set
        }
    }
}

int BITSET::GetNumSets() const { return static_cast<int>(mSets.size()); }

int BITSET::GetSet(int which_set) const
{
    if (which_set < GetNumSets()) {
    return mSets.at(which_set); 
    } else {
        return 0;
    }
    }

