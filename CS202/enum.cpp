/* Program Name: Enum
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: This program generates binary strings from input given*/

#include <iostream>
#include <string>

using namespace std;

// function to make binary strings
void do_enumeration(string &s, size_t index, int n_ones)
{
    // below is to read if ur at the end
    if (index == s.length())
    {
        // below: cout if the n_ones equal zero
        if (n_ones == 0)
        {
            cout << s << endl;
        }
        return;
    }

    // stop early if statement
    int left = s.length() - index;
    if (n_ones > left || n_ones < 0)
    {
        return;
    }

    // place a zero down
    s[index] = '0';
    do_enumeration(s, index + 1, n_ones);

    // place one down
    s[index] = '1';
    do_enumeration(s, index + 1, n_ones - 1);
}

int main(int argc, char *argv[])
{
    // correct amount of command arguments
    if ((argc != 3) ? (cerr << "Usage: " << argv[0] << " length ones" << endl, 1) : 0)
        return 1;

    // read length and ones
    int length = atoi(argv[1]);
    int ones = atoi(argv[2]);

    // ensure everything is ight
    if ((length < 0 || ones < 0 || ones > length) ? (cerr << "Error: Invalid input parameters" << endl, 1) : 0)
        return 1;

    // initalize a string with '-'
    string s(length, '-');

    // start the recursion
    do_enumeration(s, 0, ones);

    return 0;
}