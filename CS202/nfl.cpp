#include <iostream>
#include <algorithm>
// Include 'algorithm' in order to utilize min and max

using namespace std;

int main () {
double passing, completions, yards, TD, inter;
double a, b, c, d, total;
// I used double for all the variables

cout << "Enter the number of passing attempts: ";
cin >>  passing;
cout << "Enter the number of completions: ";
cin >> completions;
cout << "Enter the total passing yards: ";
cin >> yards;
cout << "Enter the number of touchdown passes: ";
cin >> TD;
cout << "Enter the number of interceptions: ";
cin >> inter;

// a = ((completions / passing) - 0.3) * 5;
a = max(0.0, min(2.375, ((completions / passing) - 0.3) * 5));
b = max(0.0, min(2.375, ((yards / passing) - 3) * 0.25));
c = max(0.0, min(2.375, (TD / passing) * 20));
d = max(0.0, min(2.375, 2.375 - (inter / passing) * 25));
total = ((a + b + c + d) / 6) * 100;

cout << "The quarterback's passer rating is " << total << endl;

}