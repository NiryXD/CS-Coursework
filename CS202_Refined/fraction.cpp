/* Program Name: Fraction
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description:  This program can multiply and divide numbers, factorials, and binomials*/

#include "fraction.hpp"
#include <iostream>

using namespace std;

// Clears all elements from functions in the header file
void Fraction::Clear()
{
    numerator.clear();
    denominator.clear();
}

// Multiplies the fraction by "n"
// Adds "n" to the numerator, unless it's already in the denominator, in which case it removes it.
// If "n" is 1, nothing changes because multiplying by 1 has no effect.
bool Fraction::Multiply_Number(int n)
{
    if (n <= 0)
        return false; // Only takes positives
    if (n == 1)
        return true; // Nothing if 1

    denominator.find(n) != denominator.end() ? denominator.erase(denominator.find(n)) : numerator.insert(n);
    return true;
    // Above is a ternary operator that checks if "n" is on the bottom. Removes it if it is, else add "n" to numerator
}

// Divides the fraction by a number "n"
bool Fraction::Divide_Number(int n)
{
    if (n <= 0)
        return false; // Only takes positives
    if (n == 1)
        return true; // Nothing if 1
    numerator.find(n) != numerator.end() ? numerator.erase(numerator.find(n)) : denominator.insert(n);
    return true;
    // Above is a ternary operator that checks if "n" is in the numerator. Removes it if it is, else adds "n" to the denominator
}

// Multiplies the fraction by "n!"
bool Fraction::Multiply_Factorial(int n)
{
    if (n <= 0)
        return false; // Only takes positives

    for (int i = 2; i <= n; ++i)
    {
        Multiply_Number(i); // Multiply by each number from 2 to "n"
    }
    return true;
}

// Divides the fraction by "n!"
bool Fraction::Divide_Factorial(int n)
{
    if (n <= 0)
        return false; // Only takes positives

    for (int i = 2; i <= n; ++i)
    {
        Divide_Number(i); // Divide by each number from 2 to "n"
    }
    return true;
}

// Multiplies the fraction by a binomial coefficient
bool Fraction::Multiply_Binom(int n, int k)
{
    if (n <= 0 || k < 0 || k > n)
        return false;

    for (int i = n - k + 1; i <= n; ++i)
    {
        Multiply_Number(i); // Multiply numerators
    }
    for (int i = 2; i <= k; ++i)
    {
        Divide_Number(i); // Divide denominators
    }
    return true;
}

// Divides the fraction by a binomial coefficient
bool Fraction::Divide_Binom(int n, int k)
{
    if (n <= 0 || k < 0 || k > n)
        return false;

    for (int i = n - k + 1; i <= n; ++i)
    {
        Divide_Number(i); // Divide numerators
    }
    for (int i = 2; i <= k; ++i)
    {
        Multiply_Number(i); // Multiply denominators
    }
    return true;
}

// Swaps the top and bottom
void Fraction::Invert()
{
    swap(numerator, denominator);
}

// Prints the fraction
void Fraction::Print() const
{
    if (numerator.empty() && denominator.empty())
    {
        // Couts 1 if the frac is empty
        cout << "1";
    }
    else
    {
        if (!numerator.empty())
        {
            // If it aint empty it iterates
            for (auto it = numerator.begin(); it != numerator.end(); ++it)
            {
                // The auto key word designates a primitive type, so like double or int
                if (it != numerator.begin())
                    cout << " * ";
                // If its not the first in line but "*"
                cout << *it;
                // "*it" is basically printing the current iteration, it's called a dereferenced
            }
        }
        else
        {
            cout << "1"; // Print one if empty
        }

        if (!denominator.empty())
        {
            // Basically the same from before, but for "/"
            cout << " / ";
            for (auto it = denominator.begin(); it != denominator.end(); ++it)
            {
                if (*it == 1)
                    continue;
                if (it != denominator.begin())
                    cout << " / ";
                cout << *it;
            }
        }
    }
    cout << endl;
}

// Multiplies all the numbers in the numerator and dividing by all the numbers in the denominator.
double Fraction::Calculate_Product() const
{
    long double num_product = 1.0;
    long double denom_product = 1.0;

    for (int n : numerator)
    {
        num_product *= n; // Multiply all numbers on the top
    }
    for (int d : denominator)
    {
        denom_product *= d; // Multiply all numbers on the bottom
    }

    return static_cast<double>(num_product / denom_product); // Return value
}
