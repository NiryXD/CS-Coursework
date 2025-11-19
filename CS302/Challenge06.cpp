/* Program Name: Challenge 6
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description: Finds DNA strands and lists them in alphabetical order */

#include <bits/stdc++.h>

using namespace std;

void findStrand(const string &s)
{
    map<string, int> AGAIN;
    // Map to track sequence appearances
    set<string> nAGAIN;
    // Set for unique ones

    int length = s.size();
    if (length < 9)
    {
        // Invalid if less than nine
        cout << "-1" << endl;
        return;
    }

    for (int i = 0; i <= length - 9; i++)
    {
        string sub = s.substr(i, 9);
        // Take note of the 9 chars
        if (AGAIN.find(sub) != AGAIN.end())
        {
            // if it's in the map already, it's AGAIN time
            AGAIN[sub] = AGAIN[sub] + 1;
        }
        else
        {
            // Start counting if it aint
            AGAIN[sub] = 1;
        }

        // Add to the map
        if (AGAIN[sub] == 2)
        {
            nAGAIN.insert(sub);
            // Add to the set
        }
    }

    set<string>::iterator iterator;
    for (iterator = nAGAIN.begin(); iterator != nAGAIN.end(); iterator++)
    {
        cout << *iterator << endl;
        // Cout in alphabetical order
    }

    cout << "-1" << endl;
}

int main()
{
    string dna;
    // Run the function made prior
    while (getline(cin, dna))
    {
        findStrand(dna);
    }
    // Look how clean this main is !!!
    return 0;
}
