/* Program Name: Printing Given Variables
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description: Taking in variables from the user and then outputting print statements that follow the given rubric.
 * This program in particular edits the given numbers.*/

//! Remember: your comments Remember: your
//! formatting and indentation
//  - auto-format in vim: gg=G in normal mode, in vscode: alt+shift+f
//! Remember: check your solution with the gradescripts
//  - gradescript command: `python3.11 scripts/test.py penname.cpp`

#include <iostream>
#include <string>

using namespace std;

int main() {

    //Initialze all variables, this will all the inputs from the user.
    int age;
    string nam;
    string name2;
    int street1;
    string city;
    string street2;
    string street3;

    //Ask Questions, print the questions so the user understands what's happening
    cout << "Enter your first and middle names:";
    cin >> nam >> name2;

    cout <<"Enter your age:";
     cin >> age;

    cout << "Enter your street number, name, and type:";
    cin >> street1 >> street2 >> street3;

    cout << "Enter your city of birth:";
    cin >> city;

    // New Numbers, they're altered to fit the format
    int nage = (street1 % age) * 3;
    int nstreet = (age * 425) / street1;

    // Print all statements, to follow the rubric
    cout << "Your penname name is " << city << " " << nam << endl;
    cout << "You will write as a " << nage << " " << "year old" << endl;
    cout << "Your address is " << nstreet << " " << name2 << " " << street3 << endl;
        return 0;
}
