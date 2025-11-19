// Program Name: Superball Analyze
// Student Name: Ar-Raniry Ar-Rasyid
// Net ID: jzr266
// Student ID: 000-663-921
// Program Description: Program to recognize the board state of Superball

#include <bits/stdc++.h>
#include "disjoint.h"

using namespace std;

class Superball
{
public:
    Superball(int argc, char **argv);
    void analyze();

    int c, r, min, empty;
    // Columns, rows, minimum score size (Usually 5), empty squares
    vector<int> board;
    // Board state
    vector<int> goal;
    // Where the goal are
    vector<int> color;
    // For point prioritization
};

// To flag the user if the input is wonky
void usage(const char *s)
{
    cout << "usage: sb-analyze rows cols min-score-size colors" << endl;
    if (s != NULL)
        // Outputs error message
        cout << s << endl;
    exit(1);
    // I had return 1; before and it didn't work
}

Superball::Superball(int argc, char **argv)
{

    // Make sure the requirements are met
    if (argc != 5)
    {
        usage(NULL);
        // If not exit
    }

    try
    {
        r = stoi(argv[1]);
        // Convert string of rows to integer of rows
        if (r <= 0)
            usage("Bad rows");
        // Output if less than 1
    }
    catch (...)
    {
        // Use if try fails
        usage("Bad rows");
    }

    // Exactly the same thing but for columns
    try
    {
        c = stoi(argv[2]);
        // Convert string of columns to integer of columns
        if (c <= 0)
            usage("Bad cols");
        // Output if less than 1
    }
    catch (...)
    {
        // Use if try fails
        usage("Bad cols");
    }

    // Exactly the same thing but for minimum score size
    try
    {
        min = stoi(argv[3]);
        // I just want to mention, I lost sleep on this becausae I wrote argv[2] instead of argv[3]. It's a bit funny in hindsight
        if (min <= 0)
            usage("Bad cols");
        // Output if less than 1
    }
    catch (...)
    {
        // Use if try fails
        usage("Bad cols");
    }

    // Checks if color is valid
    color.resize(256, 0);

    // Get input and translate from letter to number
    for (int i = 0; i < static_cast<int>(strlen(argv[4])); i++)
    {
        if (!isalpha(argv[4][i]))
            // Make sure the color is a letter
            usage("Colors must be distinct letters");
        if (!islower(argv[4][i]))
            // Make sure it's lowercase
            usage("Colors must be lowercase letters");
        if (color[argv[4][i]] != 0)
            // Check for dupes
            usage("Duplicate color");

        // Give number to color based on the dyanmic array from earlier
        color[argv[4][i]] = 2 + i;
        color[toupper(argv[4][i])] = 2 + i;
    }

    // Initalize board state
    board.resize(r * c);
    goal.resize(r * c, 0);
    empty = 0;

    // Reads in input and adjusts board state accordingly. Or throws an error
    string input;
    int i;
    int j;
    // Loop through the rows
    for (i = 0; i < r; i++)
    {
        if (!(cin >> input))
        {
            // If input fails, throw an error
            cout << "Bad board: not enough rows on standard input" << endl;
            // Here exit is used to instantly stop the process
            exit(1);
        }
        if (static_cast<int>(input.size()) != c)
        {
            // Throw an error if row length is messed up
            cout << "Bad board on row " << i << " - wrong number of characters." << endl;
            exit(1);
        }
        for (j = 0; j < c; j++)
        // Iterate over each char in the row
        {
            if (input[j] != '*' && input[j] != '.' && color[input[j]] == 0)
            {
                // Must be acceptable chars or throw an error
                cout << "Bad board row " << i << " - bad character " << input[j] << "." << endl;
                exit(1);
            }
            board[i * c + j] = input[j];
            // Below counts current empty space and increments
            if (board[i * c + j] == '.' || board[i * c + j] == '*')
                empty++;
            // Below analyzes goal state
            if (isupper(board[i * c + j]) || board[i * c + j] == '*')
            {
                goal[i * c + j] = 1;
                board[i * c + j] = tolower(board[i * c + j]);
            }
        }
    }
}

void Superball::analyze()
{
    DisjointSetByRankWPC disjoint(r * c);

    // Below loops through the board
    for (int i = 0; i < r; i++)
    {
        for (int j = 0; j < c; j++)
        {
            int index = i * c + j;

            // Recognize empties and continue
            if (board[index] == '.' || board[index] == '*')
                continue;

            // Implementation for checking neighbor
            if (j + 1 < c && board[index] == board[index + 1])
            {
                int parentOne = disjoint.Find(index);
                int parentTwo = disjoint.Find(index + 1);
                if (parentOne != parentTwo)
                    disjoint.Union(parentOne, parentTwo);
            }

            // Further implementation for checking neighbor
            if (i + 1 < r && board[index] == board[index + c])
            {
                int parentOne = disjoint.Find(index);
                int parentTwo = disjoint.Find(index + c);
                if (parentOne != parentTwo)
                    disjoint.Union(parentOne, parentTwo);
            }
        }
    }

    unordered_map<int, int> connections;
    // Map to recognize chains
    unordered_map<int, pair<int, int>> goalArea;
    // Map for goal Area

    for (int i = 0; i < r * c; i++)
    // Loop throught the board
    {
        if (board[i] == '.' || board[i] == '*')
            continue;
        // Skips
        int lead = disjoint.Find(i);
        connections[lead] = connections[lead] + 1;
        // Start at one if it just discovered a leader square.

        if (goal[i])
        // Keep the square in mind
        {
            goalArea[lead] = make_pair(i / c, i % c);
        }
    }

    cout << "Scoring sets:" << endl;
    for (unordered_map<int, int>::iterator it = connections.begin(); it != connections.end(); ++it)
    {
        int root = it->first;
        // If the chain is at least min and in goal area mark it
        if (it->second >= min && goalArea.find(root) != goalArea.end())
        {
            int row = goalArea[root].first;
            int col = goalArea[root].second;
            cout << "  Size: " << it->second << "  Char: " << static_cast<char>(board[root])
                 << "  Scoring Cell: " << row << "," << col << endl;
        }
    }
}

int main(int argc, char **argv)
{
    // Runs analyze, look how clean main is :D
    Superball s(argc, argv);
    s.analyze();
    return 0;
}
