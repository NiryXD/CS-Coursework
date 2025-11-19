/* Program Name: Checkerboard
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: Creating a checkerboard of letters through the given inputs */
#include <iostream>

using namespace std;

void line (int column, int width, char character, int cycle, int offset) {
    char neocharacter;
    // Above is a place holder for the character that can change on a whim
    for (int j = 0; j < column; j++) {
        neocharacter = character + (offset +j) % cycle;
        // Above I'm using "offset" to see how the following character need to look
        for (int k = 0; k < width; k++) {
            cout << neocharacter;
        // Right here I'm utilizing width to match the correct width
        }
    }
    cout << endl;
}

void block (int row, int column, int width, char character, int cycle) {
    for (int i = 0; i < row; i++) {
        line (column, width, character, cycle, i);
    // right here I determine how the row will look
    // also I use the "line" object I created before

        for (int k = 1; k < width; k++) {
            line (column, width, character, cycle, i);
        }
    }
}

int main () {
    int row, column, cycle, width;
    char character;

    /* names based off the the lab website. Easier naming 
    convention compared to R, C, SC, CS. */

    cin >> row >> column >> character >> cycle >> width;
    
    // Read inputs given

    block(row, column, width, character, cycle);
    // utilize the object created above to output what I want
    return 0;
}