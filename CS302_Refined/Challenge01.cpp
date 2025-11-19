/* Program Name: Challenge 1
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description: Read in user input (n, r, and d) to left or right shift integers */

#include <iostream>
#include <vector>

using namespace std;

int main()
{
    int n, r;
    char d;
    // Initalize the inputs given

    while (cin >> n >> r >> d)
    {
        vector<int> array(n);
        for (int i = 0; i < n; i++)
        {
            cin >> array[i];
        }
        // Reading in the user input and then creating a vector to store them in

        r %= n;
        // above we can avoid unneeded shifts. For example with a vector of 4 that we want to left shift 4
        // it's basically shifting 0 times because it returns to it's original state
        if (d == 'L')
        // To recognize if the user denotes left shift
        {
            vector<int> holder(array.begin(), array.begin() + r);
            for (int i = 0; i < n - r; i++)
            // Created a temporary vector to store the head
            {
                array[i] = array[i + r];
            }
            // shift everything else
            for (int i = 0; i < r; i++)
            {
                array[n - r + i] = holder[i];
            }
            // insert saved head from earlier
        }
        else if (d == 'R')
        {
            // does the same thing but shifting to the right side
            vector<int> holder(array.end() - r, array.end());
            for (int i = n - 1; i >= r; i--)
            {
                array[i] = array[i - r];
            }
            for (int i = 0; i < r; i++)
            {
                array[i] = holder[i];
            }
        }

        for (int i = 0; i < n; i++)
        {
            // Prints the arrayay with adequate spacing
            if (i > 0)
                cout << " ";
            cout << array[i];
        }
        cout << endl;
    }
}
