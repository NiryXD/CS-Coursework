/* Program Name: Challenge 3
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description: It checks if a word is a palindrome, every permutation though. Not just forwards and backwards*/

#include <iostream>
#include <unordered_map>
#include <cctype>

using namespace std;

// Function to check if it's a palindrome
bool is_palindrome(const string &s)
{
    unordered_map<char, int> chars;
    // stores letters inna map
    int odds = 0;
    // Checks for odds, since odds equal not a palindrome (Most times)

     // Run through the chars
    for (size_t i = 0; i < s.length(); i++)
    {
        char character = s[i];
        // Get current char

        if (isalpha(character))
        // Alphabet chars only
        {                              
            char lowerCase = tolower(character); // Caps and lowercase are both recognized
            chars[lowerCase]++;
            // Increases count for "this" letter in the map

            // Checks if letters are in an odd or even amount
            if (chars[lowerCase] % 2 == 0)
            {
                odds--; // minuses if it's even 
            }
            else
            {
                odds++; // add if it's odd 


            }
        }
    }

    // if odds <= 1 we're chillin
    return odds <= 1;
}

int main()
{
    string word;

    while (getline(cin, word))
    // if else statement to run the function and output to user
    {
        if (is_palindrome(word))
        {
            cout << "\"" << word << "\" is a palindrome permutation" << endl;
        }
        else
        {
            cout << "\"" << word << "\" is not a palindrome permutation" << endl;
        }
    }

    return 0;
}
