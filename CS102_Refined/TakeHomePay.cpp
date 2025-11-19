/* Program Name: Take Home Pay Calculator
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description: Basic calculator that takes in user input and converts it to the spendable amount */

#include <iomanip> // if using iomanip instead of printf
#include <iostream>
// I added in the rest of the #includes I know just in case
#include <vector>
#include <string>
#include <limits>
#include <fstream>
#include <sstream>

using namespace std;

const double MINIMUM_WAGE = 15080;
const double BRACKET1 = 10275;
const double BRACKET1_RATE = 0.10;
// Initalized more bracket variables. I utilized a shorter naming scheme so typing out my algorithm would be easier.
double b2 = 41775;
double b3 = 89075;
double b4 = 170050;
double b5 = 215950;
double b6 = 539900;
double b2r = 0.12;
double b3r = 0.22;
double b4r = 0.24;
double b5r = 0.32;
double b6r = 0.35;
double b7r = 0.37;

double Math(double salary)
{
  double tax = 0.0;
  double taxi = 0.0; // Income Tax

  /*The idea with the initalizing tax so so that it can be tailored to what suits the number the user gives the program.
  Below is a filtering system to where the user's input can be broken down to it's perspective categories. To run you through one stage of
  the process, the algoritm detects which if it meets the requirments and if it does, it adjusts the aformentioned variable, tax.
  Pretty repetitive, but I just went with what I know best. Int Main came first but I put this calculation before it since I was having trouble with
  int main. I just needed a change of pace, so I just rearranged the code. Sorry if it's confusing this way.*/

  if (salary > b6)
  {
    taxi = salary - b6;
    tax += taxi * b7r;
    salary = b6;
  }
  if (salary > b5)
  {
    taxi = salary - b5;
    tax += taxi * b6r;
    salary = b5;
  }
  if (salary > b4)
  {
    taxi = salary - b4;
    tax += taxi * b5r;
    salary = b4;
  }
  if (salary > b3)
  {
    taxi = salary - b3;
    tax += taxi * b4r;
    salary = b3;
  }
  if (salary > b2)
  {
    taxi = salary - b2;
    tax += taxi * b3r;
    salary = b2;
  }
  if (salary > BRACKET1)
  {
    taxi = salary - BRACKET1;
    tax += taxi * b2r;
    salary = BRACKET1;
  }

  taxi = salary;
  tax += taxi * BRACKET1_RATE;
  return tax;
}

int main()
{
  double salary;
  cout << "Enter a salary: $";
  cin >> salary;

  // Filled out the commented out boxes. The line of reasoning behind this code is to not calculate values that aren't big enough to tax.
  // So I took the value of "MINIMUM_WAGE" pre-establised from earlier to set the bar.
  if (salary < MINIMUM_WAGE)
  {
    cerr << "This is less than minimum wage for a yearly salary." << endl;
    return 1;
  }

  // Getting the easier calculations out the way. The percentages were given so I just put them in.
  double med = salary * 0.0145;
  double ss = salary * 0.062;

  // After figuring out the net pay we're able to find the other taxes required in the print statement. Again I used shortened variable names
  // for the sake of convinence. This is when my calculator comes in handy. An explanation is found further up the code.
  double ntaxi = Math(salary);
  double take = salary - ss - med - ntaxi;
  double month = take / 12;

  /* Onto what I believe was the hardest part is the formatting. It looks funny this way since I kept changing the numbers and having them
  parted like this is easier on the eyes. Actually it's gone now, since I formatted it. I just had to keep changing the numbers, and when 
  I realized they were on the assignment page I was mad. Very mad.*/

  cout << setprecision(2) << fixed;
  cout << "-----------------------------------" << endl;
  cout << left << setw(25) << "Yearly Salary:" << right << "$" << setw(9) << salary << endl;
  cout << left << setw(25) << "Social Security Tax:" << right << "$" << setw(9) << ss << endl;
  cout << left << setw(25) << "Medicare Tax:" << right << "$" << setw(9) << med << endl;
  cout << left << setw(25) << "Income Tax:" << right << "$" << setw(9) << ntaxi << endl;
  cout << "-----------------------------------" << endl;
  cout << left << setw(25) << "Take Home Salary:" << right << "$" << setw(9) << take << endl;
  cout << left << setw(25) << "Monthly Take Home Pay:" << right << "$" << setw(9) << month << endl;

  return 0;
}
