/* Program Name: Miles Per Gallon
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description: From a fleet of cars and trucks, this program predicts it's the collective car's and truck's milage */

#include <iomanip>
#include <iostream>
// copy and pasted from the previous program. Who knows, I might need all of them
#include <vector>
#include <string>
#include <limits>
#include <fstream>
#include <sstream>

using namespace std;

// Here you can see me initalizing all the variables that I'll need throughout the program.

int main (){
    double CarM = 0;
    double CarG = 0;
    double TruckM = 0;    
    double TruckG = 0;
    int numCar = 0;
    int numTruck = 0;

/* Basiclly there's this big funnel that takes in the first input. There are four choices: car, truck, gibberish, or done. This is completed
by a while statement because when the user types in done the loop breaks. But while the loop is still in play and the user chooses 
car or truck it will ask them the corresponding questions. The code for car and truck are identical they just hold different variables
to seperate the two.*/

while (true) {
    cout << "Enter command: ";
    string input;
    cin >> input;
    // I had trouble with cin.ignore(); so I googled it, and this abomination came up. It utilizes limits, and without it the text
    // would keep repeating. I utilized it all through my code.
    cin.ignore(numeric_limits<streamsize>:: max(), '\n');

    if (input == "car") {
        double g, m;
        cout << "Enter car's miles: ";
        while (!(cin >> m)) {
            cout << "Enter car's miles: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>:: max(), '\n');
        }

        cout << "Enter car's gallons: ";
        while (!(cin >> g)) {
            cout << "Enter car's gallons: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>:: max(), '\n');
        }
        // This part adjusts variables made prior. It helps us get the number we want in the final calculations.
        CarM += m;
        CarG += g;
        numCar++;
    }

    // exactly like the line of code for car, just replaced with truck. 

    else if (input == "truck") {
        double g, m;
        cout << "Enter truck's miles: ";
        while (!(cin >> m)) {
            cout << "Enter truck's miles: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>:: max(), '\n');
        }

        cout << "Enter truck's gallons: ";
        while (!(cin >> g)) {
            cout << "Enter truck's gallons: ";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>:: max(), '\n');
        }

        TruckM += m;
        TruckG += g;
        numTruck++;
    }

    // This is what I was talking about earlier, with how when the user types in done it breaks the loops. Therefore continuing
    // to the final step. Which is giving the results to the user. And also there's the "Unknown command" prompt that acknowledges
    // unreadable input

    else if (input == "done") {
        break;
    }
    else {
        cout << "Unknown command." << endl;
    }
}
 
/* Based on the adjustments made earlier. Mixes some numbers around and prints it out to the user, so it's bite-sized for the 
the reader. */

if (numCar == 0 ) {
    cout << "Fleet has no cars." << endl;
}
else {
    cout.precision(6);
    cout << "Average car MPG = " << CarM / CarG << endl;
}

if (numTruck == 0 ) {
    cout << "Fleet has no trucks." << endl;
}
else {
    cout.precision(6);
    cout << "Average truck MPG = " << TruckM / TruckG << endl;
}
}

// Not really sure if I can include "return 0;", When I included it, I had an error. 