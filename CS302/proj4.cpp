/* 
 * Program Name: Project 4  
 * Student Name: Ar-Raniry Ar-Rasyid  
 * Net ID: jzr266  
 * Student ID: 000-663-921  
 * Program Description: Create a map that Dijkstra's can navigate 
 */

 #include <bits/stdc++.h>
 
 using namespace std;
 
 int main(int argc, char* argv[]) {
     int N;
 
     // Takes un user input
     if (argc > 1) {
         N = stoi(argv[1]);
     }
 
     // Get a random number, help from w3schools.com
    srand(time(0));
 
     // Constants for tiles and their weights
     const char tileLetter[] = {'f', 'g', 'G', 'h', 'm', 'r'};
     const int tileNumber[] = {3, 1, 2, 4, 7, 5};
 
     // Cout tile info
     cout << "6" << endl;
     for (int i = 0; i < 6; i++) {
         cout << tileLetter[i] << " " << tileNumber[i] << endl;
     }
 
     // Cout the graph the user wanted
     cout << N << " " << N << endl;
 
     // Generate random map
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int randTile = rand() % 6;
            // Picks a number between 0 - 5, then chooses the corresponding tile
            cout << tileLetter[randTile];

            if (j < N - 1) {
                cout << " ";
            }
        }
        cout << endl;
    }
 
     // Start at top left
     cout << "0 0" << endl;
 
     // End at bottom right
     cout << N - 1 << " " << N - 1 << endl;
 
     return 0;
 }
 