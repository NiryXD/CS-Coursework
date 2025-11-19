/* Program Name: Keno
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: Calculates the expected return of a Keno bet. */

#include "fraction.hpp"
#include <iostream>
#include <iomanip>

using namespace std;

int main()
{
    double bet;
    // Stores moniez
    int picked;
    // Number of balls picked

    cin >> bet >> picked;

    // Print the bet and number of balls picked
    printf("Bet: %.2f\n", bet);
    // Float that specifies two decimal places (above)
    cout << "Balls Picked: " << picked << endl;
    // Print balls picked (above)

    Fraction total_fraction;
    // Initialize fraction
    double total_expected_return = 0.0;
    // To keep track of total

    int catch_number;
    // store numbers caught
    double payout;
    // Variable to store the payout for catching a certain number of balls

    while (cin >> catch_number >> payout)
    {
        Fraction prob_fraction;
        // Create a new fraction object for calculating probabilities

        // Calculate the probability of catching exactly 'catch_number' out of 'picked'
        prob_fraction.Multiply_Binom(80 - picked, 20 - catch_number);
        prob_fraction.Multiply_Binom(picked, catch_number);
        prob_fraction.Divide_Binom(80, 20);
        // Above multiplies different binomial coefficients

        // Calculate the final probability
        double prob = prob_fraction.Calculate_Product();
        // Calculate the expected return
        double expected_return = prob * payout;

        // Print the probability of catching 'catch_number' balls
        cout << "  Probability of catching " << catch_number << " of " << picked << ": " << prob << " -- Expected return: " << expected_return << endl;

        total_expected_return += expected_return;
    }

    double return_per_bet = total_expected_return - bet;
    // Return per bet is total minus initial
    double normalized_return = return_per_bet / bet;
    // Normalized return is the return divided by bet

    // Print the final return
    printf("Your return per bet: %.2f\n", return_per_bet);
    printf("Normalized: %.2f\n", normalized_return);
    // You like my snake case? Been practicing python

    return 0;
}
