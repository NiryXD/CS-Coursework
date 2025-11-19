#include <iostream>
#include <limits>

using namespace std;

// Function to get a valid numeric input
double getValidNumericInput(const string& prompt) {
    double value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            break;
        } else {
            cout << "Invalid input. Please enter a valid number." << endl;
            cin.clear();
            cin.ignore();
        }
    }
    return value;
}

int main() {
    double totalCarMiles = 0;
    double totalCarGallons = 0;
    int numCars = 0;

    double totalTruckMiles = 0;
    double totalTruckGallons = 0;
    int numTrucks = 0;

    string command;

    while (true) {
        cout << "Enter command: ";
        cin >> command;

        if (command == "car") {
            double miles = getValidNumericInput("Enter car's miles: ");
            double gallons = getValidNumericInput("Enter car's gallons: ");

            totalCarMiles += miles;
            totalCarGallons += gallons;
            numCars++;
        } else if (command == "truck") {
            double miles = getValidNumericInput("Enter truck's miles: ");
            double gallons = getValidNumericInput("Enter truck's gallons: ");

            totalTruckMiles += miles;
            totalTruckGallons += gallons;
            numTrucks++;
        } else if (command == "done") {
            break;
        } else {
            cout << "Unknown command." << endl;
        }
    }

    if (numCars == 0) {
        cout << "Fleet has no cars." << endl;
    } else {
        cout.precision(6);
        cout << "Average car MPG = " << totalCarMiles / totalCarGallons << endl;
    }

    if (numTrucks == 0) {
        cout << "Fleet has no trucks." << endl;
    } else {
        cout.precision(7);
        cout << "Average truck MPG = " << totalTruckMiles / totalTruckGallons << endl;
    }

    return 0;
}
