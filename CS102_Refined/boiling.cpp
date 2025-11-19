/* Program Name: Boiling Point
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description: Takes in elavation based on location and then calculates
 pressure and boiling point of water.*/

#include <cmath> // for the math we're going to use later
#include <cstddef>
#include <fstream>
#include <iomanip> // All these copy pasted from the last program
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

double elevationToBoilingPoint(const double elevation) {

    double boilingPoint = 0.0;
    double pressure = 29.921 * pow(1 - 0.0000068753 * elevation, 5.2559); // These formulas taken from the the project page
    boilingPoint = 49.161 * log(pressure) + 44.932; // log used from the cmath pound include
    return boilingPoint;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        cerr << "usage: ./boiling filename" << endl; //error message
        return 1;
    }

    ifstream inputFile(argv[1]);
    if (!inputFile.is_open()) {
        cerr << "File failed to open." << endl;
        return 1;
    }

    vector<string> cityNames; // initalizing vectors for the .txt files
    vector<string> state;
    vector<int> altitude;
    
    string line;
    getline(inputFile, line);

    while (getline(inputFile, line)) { 
        string City; //initalizing all the collumns as strings, I can stoi later if needed
        string State;
        string Number;

        istringstream recordStream(line);

        getline(recordStream, City, ','); // they're divided by commas, hence the usage of ','
        getline(recordStream, State, ',');
        getline(recordStream, Number, ',');

        cityNames.push_back(City);
        state.push_back(State);
        altitude.push_back(stoi(Number)); // Here's the stoi mentioned earlier
    }

    inputFile.close();

    cout << "Boiling Point at Altitude Calculator" << endl;
    for (size_t i = 0; i < cityNames.size(); ++i) { //Just a loop to paste in the City names in order
        cout << i + 1 << ". " << cityNames[i] << endl;
    }

    int cityNumber;
    cout << "Enter city number: "; 
    cin >> cityNumber; // Above asks the user for input, which is either 1-18. 
    int altitudeNumber = cityNumber; 

    if (cityNumber < 0 || cityNumber > cityNames.size()) {
        cerr << "Invalid city number." << endl; // the error message shown if junk was inputted, and above that is what recognizes if it's a valid number or not
        return 1;
    }

    double boilingPoint = elevationToBoilingPoint((altitude[cityNumber - 1]));
    cout << "The boiling point at " << cityNames[cityNumber - 1] << " is "
        << fixed << setprecision(3) << boilingPoint << " degrees Fahrenheit." /
        << endl; // Above is prints the correct output. Where the city aswell as the correct tempurature is shown. 

    return 0;
}