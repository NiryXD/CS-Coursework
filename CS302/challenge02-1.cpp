/* Program Name: Challenge 2
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description:  Takes a list of integers and gives you ints with the smallest difference. Shows multiple if avalible*/

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char *argv[])
{
    int Num;

    // To read in all the inputs
    while (cin >> Num)
    {

        // Store the Numbers in a vector
        vector<int> Number(Num);

        for (int i = 0; i < Num; i++)
        {
            cin >> Number[i];
            // Above puts em into an array
        }

        sort(Number.begin(), Number.end());
        // Above sorts em in acending order

        int difference = Number[Num - 1] - Number[0];
        // Get largest number n den subtract by the smallest

        for (int i = 0; i < Num - 1; i++)
        {
            // Updates whatever is the current contender
            difference = min(difference, abs(Number[i + 1] - Number[i]));
        }

        // Outputs for the user
        bool pair = true;
        for (int i = 0; i < Num - 1; i++)
        {
            if (abs(Number[i + 1] - Number[i]) == difference)
            {
                // Check for if also the lowest
                if (!pair)
                    cout << " ";
                // Adds a space if it aint the first pair printed
                if (Number[i] == Number[i + 1])
                {
                    cout << Number[i] << " " << Number[i];
                }
                // Print if both numbers are the same
                else
                {
                    cout << Number[i] << " " << Number[i + 1];
                    // print if diff numbers thats in a pair
                }
                pair = false;
            }
        }
        // Go one a new line
        cout << endl;
    }
    return EXIT_SUCCESS;
}