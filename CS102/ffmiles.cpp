#include <iostream>
#include <limits>

using namespace std;

int main() {
    double totalCarMiles = 0.0, totalCarGallons = 0.0, totalTruckMiles = 0.0, totalTruckGallons = 0.0;
    int numCars = 0, numTrucks = 0;

    while (true) {
        string command;
        cout << "Enter command (car, truck, done): ";
        cin >> command;
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear input buffer

        if (command == "car") {
            double miles, gallons;
            cout << "Enter car's miles: ";
            if (!(cin >> miles)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a valid number for miles.\n";
                continue;
            }
            cout << "Enter car's gallons: ";
            if (!(cin >> gallons)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a valid number for gallons.\n";
                continue;
            }
            totalCarMiles += miles;
            totalCarGallons += gallons;
            numCars++;
        } else if (command == "truck") {
            double miles, gallons;
            cout << "Enter truck's miles: ";
            if (!(cin >> miles)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a valid number for miles.\n";
                continue;
            }
            cout << "Enter truck's gallons: ";
            if (!(cin >> gallons)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Invalid input. Please enter a valid number for gallons.\n";
                continue;
            }
            totalTruckMiles += miles;
            totalTruckGallons += gallons;
            numTrucks++;
        } else if (command == "done") {
            if (numCars == 0)
                cout << "Fleet has no cars.\n";
            else
                cout << "Average car MPG = " << totalCarMiles / totalCarGallons << endl;

            if (numTrucks == 0)
                cout << "Fleet has no trucks.\n";
            else
                cout << "Average truck MPG = " << totalTruckMiles / totalTruckGallons << endl;

            break;
        } else {
            cout << "Unknown command.\n";
        }
    }

    return 0;
}
