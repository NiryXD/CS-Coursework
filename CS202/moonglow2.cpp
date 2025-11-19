/* Program Name: Moonglow
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: Data organization and comprehension through strange inputs */
#include <iostream>
#include <string>
#include <cctype>

using namespace std;

int main () {
    string word;
    string name;
    double total = 0.0;
    bool average = false;
    double sum = 0.0;
    int count = 0;
    // initating all the words I want to use in the following program

    while (cin >> word) {
        if (word == "NAME") {
            cin >> name;
        }
        // right here I can check for "NAME"
        else if (word == "AVERAGE") {
            average = true;
            sum = 0.0;
            count = 0;
        }
        // Right here I check for "AVERAGE" aswell as reset the counters I have initated
        else if (average && isdigit(word[0])) {
            sum += stod(word);
            count++;
        }
        // If all goes according to plan we convert the word to a double and add it to sum
        // and we increment the count counter
        else if (average && !isdigit(word[0])) {
            if (count > 0 ) {
                total += sum / count;
            }
            average = false;
        }
        else if (isdigit(word[0])) {
            total += stod(word);
        }
    }
    // right here is just adding to the total

    if (average && count > 0) {
        total += sum / count;
    }
    // It's no longer a if else loop here, so it's it's own thing
    // just a final calculation of the average

    cout << name << " " << total << endl;
    // printing everything to be shown
}