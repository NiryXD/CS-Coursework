/* Program Name: Speeding Ticket Calculator
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description: Charging the user money based on their location*/

#include <iomanip> // All these copy pasted from the last program
#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <fstream>
#include <sstream>

using namespace std;

const double INTERSTATE_MULTIPLIER = 5.2243;
const double HIGHWAY_MULTIPLIER = 9.4312;
const double RESIDENTIAL_MULTIPLIER = 17.2537;
const double DEFAULT_MULTIPLIER = 12.916;

// Above I just pasted all the given doubles on the program page. Numbers need to be exact or the calculations are going to be wrong.

const string MONTH_NAMES[] = {"Err", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// This are just an arry of the month, aswell as Err standing for error.

string citationNumber, fineValue;
int day, month, year;
double carSpeed, speedLimit;
char roadType;

// Above is just the variables I initiatied. All related to the inputted data in some form.

int main() {
    try {
        // I experimented using a new command named 'try'. It's basically a if then statement. Just thought that was cool.
        string ticketFilename, reportFilename;
        int startDay, startMonth, startYear, endDay, endMonth, endYear;
        cout << "Enter a ticket file: ";
        cin >> ticketFilename;
        ifstream ticketFile(ticketFilename);

        if(!ticketFile) {
            cerr <<"Unable to open " << ticketFilename << "." << endl;
            return 1;
        }

        // This here is just asking the user a prompt and there's two responses. One is to continue onward, the other is to tell the
        // user, or rather police officer, that the file is not exccepted. It then breaks the line of code using return 1;.

        cout << "Enter a report file: ";
        cin >> reportFilename; // This was the bug that was submitted late. Brackets were backwards
        ofstream reportFile(reportFilename);

        if (!reportFile) {
            cerr << "Unable to open " <<reportFilename  << "." << endl;
            return 1;
        }

        // The same thing is done twice one for the ticket file and the second for the report. They are identical, just taking different
        // variables.

        cout << "Enter report start date (mm dd yyyy): ";
        cin >> startMonth >> startDay >> startYear;

        cout << "Enter report end date (mm dd yyyy): ";
        cin >> endMonth >> endDay >> endYear;

        // Another prompt for the user. This one in particular asks for the start and end dates.

        while (ticketFile >> citationNumber >> month >> day >> year >> carSpeed >> speedLimit >> roadType) {
            if (year < startYear || year > endYear ||
            (year == startYear && month < startMonth) ||
            (year == endYear && month > endMonth) ||
            (year == endYear && month == endMonth && day > endDay) ) {
                continue;
            }

            // The code above just reads the .txt files and organizes them into their respective variables. 

            double fine = 0.0;
            switch (tolower(roadType)) {
                case 'i': 
                fine = (carSpeed - speedLimit) * INTERSTATE_MULTIPLIER;
                break;
                case 'h':
                fine = (carSpeed - speedLimit) * HIGHWAY_MULTIPLIER;
                break;
                case 'r':
                fine = (carSpeed - speedLimit) * RESIDENTIAL_MULTIPLIER;
                break;
                default:
                fine = (carSpeed - speedLimit) * DEFAULT_MULTIPLIER;
            }

            // These break statements cascade until they find the correct values. Like if it wasn't a residential type road then 
            // it would skip through both interstate and highway in order to make it's way into the third slot. If all else fails
            // then it would drop down to it's default case.

            fine = max(0.0, fine);

            reportFile << right << setw(2) << setfill('0') << month << "-" << MONTH_NAMES[month] << "-" << year << " " << setw(10) <<
            left << citationNumber << " $" << fixed << setprecision(2) << " " << right << setw(8) << fine << fixed << endl;
             }

             // Above is just a print statement that fits the requirements. Arguably the hardest part, since the example was very
             // particular. Basic use of setw and setprecision.
    
    ticketFile.close();
    reportFile.close();

    // Something along the line of closing Scanners like in Java

    } catch (const exception &e) {
        cout << "Error: " << e.what() << endl;
        return 1;

        // The then statement for catch. Has no use in this program, but I was too far gone.

    }
        return 0;
        // Return zero to finish off the program :)


    }

