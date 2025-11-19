// Program Name: Superball Play
// Student Name: Ar-Raniry Ar-Rasyid
// Net ID: jzr266
// Student ID: 000-663-921
// Program Description: Algorithm to get max points for Superball

#include <bits/stdc++.h>
#include "disjoint.h"

using namespace std;

class Superball
{
public:
    Superball(int argc, char **argv);
    void play();

    int r, c, min, empty;
    vector<int> board;
    vector<int> goals;
    vector<int> color;
};

// Public basically the same in my Superball Analyze

Superball::Superball(int argc, char **argv)
{
    // I'm not sure if I should re-explain everything, but I guess I will
    r = stoi(argv[1]);
    c = stoi(argv[2]);
    min = stoi(argv[3]);
    // String to integer all the inputs

    color.resize(256, 0);
    // Checks if color is valid

    for (int i = 0; i < (int)strlen(argv[4]); i++)
    {
        // Give number to color based on the dyanmic array
        color[argv[4][i]] = 2 + i;
        color[toupper(argv[4][i])] = 2 + i;
    }

    // Initalize board state
    board.resize(r * c);
    goals.resize(r * c, 0);
    empty = 0;

    string input;
    for (int i = 0; i < r; i++)
    // Read through the board
    {
        cin >> input; 
        // Read row as a string
        for (int j = 0; j < c; j++)
        {
            board[i * c + j] = input[j];

            if (board[i * c + j] == '.' || board[i * c + j] == '*')
                empty++;
            // Counts empties

            if (isupper(board[i * c + j]) || board[i * c + j] == '*')
            // Finds goals
            {
                goals[i * c + j] = 1;
                board[i * c + j] = tolower(board[i * c + j]);
            }
        }
    }
}

void Superball::play() {
    DisjointSetByRankWPC disjoint(r * c);

    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            int index = i * c + j;
            if (board[index] == '.' || board[index] == '*') 
            continue;
            // Skip empties

            // Checks right neighbors
            if (j + 1 < c && board[index] == board[index + 1])
            {
                disjoint.Union(disjoint.Find(index), disjoint.Find(index + 1));
            }

            // Bottom meighbors
            if (i + 1 < r && board[index] == board[index + c])
            {
                disjoint.Union(disjoint.Find(index), disjoint.Find(index + c));
            }
        }
    }

    unordered_map<int, int> connections;
    // Map to recognize chains
    unordered_map<int, pair<int, int>> goalArea;
    // Map for goal Area
    vector<pair<int, int>> scorePoints;

    for (int i = 0; i < r * c; i++) 
    {
        if (board[i] == '.' || board[i] == '*')
        continue;
        // Skip empties

        int lead = disjoint.Find(i);
        // Find leader square
        connections[lead]++;
        // Count its suboridnates

        if (goals[i])
        // If it's in goal Area, keep it in mind
        {
            goalArea[lead] = {i / c, i % c};
        }
    }

    for (unordered_map<int, int>::iterator iterate = connections.begin(); iterate != connections.end(); iterate++)
    {
        // Recognizing group size and the OG square
        int parent = iterate->first;
        int size = iterate->second;

        if(size >= min && goalArea.count(parent))
        {
            // Score it if it meets the minimum
            scorePoints.push_back(goalArea[parent]);
        }
    }

    if(!scorePoints.empty()) 
    {
        // If no scoring moves execute this
        pair<int, int> start = scorePoints.front();
        int r2 = start.first;
        int c2 = start.second;
        cout << "SCORE " << r2 << " " << c2 << endl;
    }
    else
    {
        // Swapping done here
        for (int i = 0; i < r * c; i++) {
            for (int j = i + 1; i < r * c; i++) {
            if (board[i] != '.' && board[j] != '*') {
                // Not gonna swap an empty
                for (int j = i + 1; j < r * c; j++) {
                    if (board[j] != '.' && board[j] != '*' && board[i] != board[j]) {
                        cout << "SWAP " << i / c << " " << i % c << " " << j / c << " " << j % c << endl;
                        return;
                    }
                }
            }
        } 
    }
}
}

int main (int argc, char **argv) {
    // Play da game
    Superball s(argc, argv);
    s.play();
    return 0;
}