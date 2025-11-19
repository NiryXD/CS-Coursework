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

    int r, c;          // Rows and columsn
    int min;           // Min chain
    int empty;         // Empty cells
    vector<int> board; // Board representation
    vector<int> goals; // Goal positions
    vector<int> color; // Char to score

private:
    struct ScoringSet
    {
        int size;
        int value;
        int row;
        int col;
        char color;
    };

    int checkSwap(int r1, int c1, int r2, int c2); // Scores a potential swap
    bool goalCheck(int row, int col);              // Next to goal or not
};

void usage(const char *s)
{
    cerr << "usage: sb-play rows cols min-score-size colors" << endl;
    if (s != NULL)
        cerr << s << endl;
    exit(1);
}

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
    {
        // Read through the board
        cin >> input;
        // Read row as a string
        for (int j = 0; j < c; j++)
        {
            board[i * c + j] = input[j];

            if (board[i * c + j] == '.' || board[i * c + j] == '*')
                empty++;
            // Counts empties

            if (isupper(board[i * c + j]) || board[i * c + j] == '*')
            {
                // Finds goals
                goals[i * c + j] = 1;
                board[i * c + j] = tolower(board[i * c + j]);
            }
        }
    }
}

bool Superball::goalCheck(int row, int col)
{
    const int rowMoves[] = {-1, 1, 0, 0};
    const int colMoves[] = {0, 0, -1, 1};

    // Chek for goal cells
    for (int i = 0; i < 4; i++)
    {
        int nr = row + rowMoves[i];
        int nc = col + colMoves[i];

        if (nr >= 0 && nr < r && nc >= 0 && nc < c)
        {
            if (goals[nr * c + nc])
            {
                return true;
            }
        }
    }
    return false;
}

int Superball::checkSwap(int r1, int c1, int r2, int c2)
{
    int index1 = r1 * c + c1;
    int index2 = r2 * c + c2;

    if (board[index1] == board[index2])
    {
        return 0;
    }
    // Testing swaps
    swap(board[index1], board[index2]);

    int value = 0;
    const int rowMoves[] = {-1, 1, 0, 0};
    const int colMoves[] = {0, 0, -1, 1};

    // evaluate both swapped positions
    for (int pos = 0; pos < 2; pos++)
    {
        int row;
        if (pos == 0)
        {
            row = r1;
        }
        else
        {
            row = r2;
        }
        int col;
        if (pos == 0)
        {
            col = c1;
        }
        else
        {
            col = c2;
        }
        int index = row * c + col;
        char current = board[index];

        int matchNeighbor = 0;
        bool goalClose = false;

        // check neighbors for match and goal proximity
        for (int d = 0; d < 4; d++)
        {
            int nr = row + rowMoves[d];
            int nc = col + colMoves[d];

            if (nr >= 0 && nr < r && nc >= 0 && nc < c)
            {
                if (board[nr * c + nc] == current)
                {
                    matchNeighbor++;
                }
                if (goals[nr * c + nc])
                {
                    goalClose = true;
                }
            }
        }

        if (matchNeighbor > 0)
        {
            int colorValue = color[current];
            int baseValue = matchNeighbor * colorValue;

            // Multiplier from color weight
            if (colorValue >= 5)
            {
                baseValue *= 2;
            }
            else if (colorValue == 4)
            {
                baseValue = baseValue * 3 / 2;
            }

            // Bonus of goal is close
            if (goals[index])
            {
                baseValue *= 3;
            }
            else if (goalClose)
            {
                baseValue *= 2;
            }

            value += baseValue;
        }
    }

    // Original
    swap(board[index1], board[index2]);
    return value;
}

void Superball::play()
{
    // Play da game
    DisjointSetByRankWPC disjoint(r * c);

    for (int i = 0; i < r; i++)
    {
        for (int j = 0; j < c; j++)
        {
            int index = i * c + j;
            if (board[index] == '.' || board[index] == '*')
                continue;
            // Skip empties

            // Union with right neighbor if same color
            if (j + 1 < c && board[index] == board[index + 1])
            {
                disjoint.Union(disjoint.Find(index), disjoint.Find(index + 1));
            }
            // Union with bottom neighbor if same color
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

    // Count chain sizes
    for (int i = 0; i < r * c; i++)
    {
        if (board[i] == '.' || board[i] == '*')
            continue;
        // Skip empties

        int leader = disjoint.Find(i);
        // Find leader square
        connections[leader]++;
        // Count its suboridnates

        if (goals[i])
        {
            // If it's in goal Area, keep it in mind
            goalArea[leader] = {i / c, i % c};
        }
    }

    for (unordered_map<int, int>::iterator it = connections.begin(); it != connections.end(); ++it)
    {
        int parent = it->first;
        int size = it->second;

        if (size >= min && goalArea.count(parent))
        {
            // Score it if it meets the minimum
            scorePoints.push_back(goalArea[parent]);
        }
    }

    if (!scorePoints.empty())
    {
        // If no scoring moves execute this
        pair<int, int> start = scorePoints.front();
        cout << "SCORE " << start.first << " " << start.second << endl;
    }
    else
    {
        // Swapping done here
        vector<pair<int, int>> cells;
        for (int i = 0; i < r; i++)
        {
            for (int j = 0; j < c; j++)
            {
                int index = i * c + j;
                if (board[index] != '.' && board[index] != '*')
                {
                    cells.push_back({i, j});
                }
            }
        }

        int best_value = -1;
        int best_r1 = -1, best_c1 = -1, best_r2 = -1, best_c2 = -1;

        // Test every pair
        for (size_t i = 0; i < cells.size(); i++)
        {
            for (size_t j = i + 1; j < cells.size(); j++)
            {
                int r1 = cells[i].first;
                int c1 = cells[i].second;
                int r2 = cells[j].first;
                int c2 = cells[j].second;

                if (board[r1 * c + c1] == board[r2 * c + c2])
                    continue;

                int value = checkSwap(r1, c1, r2, c2);

                if (value > best_value)
                {
                    // Remember
                    best_value = value;
                    best_r1 = r1;
                    best_c1 = c1;
                    best_r2 = r2;
                    best_c2 = c2;
                }
            }
        }

        if (best_value > 0)
        {
            // Output winning swap
            cout << "SWAP " << best_r1 << " " << best_c1 << " " << best_r2 << " " << best_c2 << endl;
        }
        else
        {
            // Random
            srand(time(NULL));
            int i1 = rand() % cells.size();
            int i2;
            do
            {
                i2 = rand() % cells.size();
            } while (i1 == i2);

            cout << "SWAP "
                 << cells[i1].first << " " << cells[i1].second << " "
                 << cells[i2].first << " " << cells[i2].second << endl;
        }
    }
}

int main(int argc, char **argv)
{
    Superball s(argc, argv);
    s.play();
    return 0;
}
