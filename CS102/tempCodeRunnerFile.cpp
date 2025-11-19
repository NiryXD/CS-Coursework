#include <iostream>
using namespace std;

int main() {

  /*  char letter;
  int i = 0;

  while (cin >> letter) {
    if (letter == 'A')
      i++;
    if (letter == 'C')
      i++;
    i++;
    i++;
    if (letter == 'Z')
      i + 26;
    cout << i << endl; */

  char letter;
  int totalGold = 0;

  while (cin.get(letter)) {
    if (letter >= 65 && letter <= 90)
      totalGold += letter - 64;
    // totalGold += letter - 'A' - 1;
    // if (letter >= 'A' && letter <= 'Z' {
  }
  cout << totalGold << endl;
}