#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

const double INTERSTATE_MULTIPLIER = 5.2243;
const double HIGHWAY_MULTIPLIER = 9.4312;
const double RESIDENTIAL_MULTIPLIER = 17.2537;
const double DEFAULT_MULTIPLIER = 12.916;

const string MONTH_NAMES[] = {"Err", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

string citationNumber;
int day, month, year;
double carSpeed, speedLimit;
char roadType;
string fineValue;

int main() {
    try {
        string ticketFilename, reportFilename;
        int startDay, startMonth, startYear, endDay, endMonth, endYear;

        cout << "Enter a ticket file: ";
        cin >> ticketFilename;
        ifstream ticketFile(ticketFilename);

        if (!ticketFile) {
            cerr << "Unable to open " << ticketFilename << "." << endl;
            return 1;
        }

        cout << "Enter a report file: ";
        cin >> reportFilename; // This was the bug that was submitted late. Brackets were backwards
        ofstream reportFile(reportFilename);

        if (!reportFile) {
            cerr << "Unable to open " << reportFilename << "." << endl;
            return 1;
        }

        cout << "Enter report start date (mm dd yyyy): ";
        cin >> startMonth >> startDay >> startYear;

        cout << "Enter report end date   (mm dd yyyy): ";
        cin >> endMonth >> endDay >> endYear;

        while (ticketFile >> citationNumber >> month >> day >> year >> carSpeed >> speedLimit >> roadType) {
            if (year < startYear || year > endYear || 
                (year == startYear && month < startMonth) || 
                (year == endYear && month > endMonth) || 
                (year == endYear && month == endMonth && day > endDay)) {
                continue;
            }

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

            fine = max(0.0, fine);


            reportFile << right << setw(2) << setprecision(2) << day << "-" << MONTH_NAMES[month] << "-" << year << " " 
                       << setw(10) << left << citationNumber << "    $" << fixed << setprecision(2) << "  " << fine << fixed << endl;
        }

        ticketFile.close();
        reportFile.close();

    } catch (const exception &e) {
        cout << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
