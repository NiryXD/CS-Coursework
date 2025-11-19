/*
 * Program Name: Challenge 8
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description: This program compares two DNA sequences and calculates how similar they are using the Needleman-Wunsch algorithm.
 */

 #include <bits/stdc++.h>

 using namespace std;

 // Function for Needleman Wunsch, below is just pulled from write up and repo
 int needlemanW(const string &seq1, const string &seq2, int match = 1, int mismatch = -1, int gap = -1)
 {

     int a = seq1.length();
     int b = seq2.length();
     // Above are just given sequencse

     // Make a matrix to store scores
     vector<vector<int>> dynamicProgram(a + 1, vector<int>(b + 1, 0));

     // Form the first column
     for (int i = 0; i <= a; i++)
     {
         dynamicProgram[i][0] = i * gap;
     }
     // Form the first row
     for (int j = 0; j <= b; j++)
     {
         dynamicProgram[0][j] = j * gap;
     }

     // Fill out the rest
     for (int i = 1; i <= a; i++)
     {
         for (int j = 1; j <= b; j++)
         {
             // If they match then reward it, else give penalty
             int compareS;
             if (seq1[i - 1] == seq2[j - 1])
             {
                 compareS = match;
             }
             else
             {
                 compareS = mismatch;
             }

             // Try all sides
             int diagonal = dynamicProgram[i - 1][j - 1] + compareS;
             int up = dynamicProgram[i - 1][j] + gap;
             int left = dynamicProgram[i][j - 1] + gap;

             // Recognize the highest score
             dynamicProgram[i][j] = max({diagonal, up, left});
         }
     }

     return dynamicProgram[a][b];
 }

 // Read in DNA seq from file
 string readSequence(const string &filename)
 {
     ifstream file(filename);
     string sequence, line;
     while (getline(file, line))
     {
         sequence = sequence + line;
     }
     return sequence;
 }

 int main(int argc, char *argv[])
 {
     // Error Message
     if (argc != 3 && argc != 1)
     {
         cerr << "Usage: " << argv[0] << " [sequence1_file sequence2_file]" << endl;
         return 1;
     }

     string seq1, seq2;

     // Two files get read
     if (argc == 3)
     {
         seq1 = readSequence(argv[1]);
         seq2 = readSequence(argv[2]);
     }
     else
     // Else throw out 0
     {
         cout << "0" << endl;
         return 0;
     }

     // Use function made
     int score = needlemanW(seq1, seq2);
     cout << score << endl;

     return 0;
 }