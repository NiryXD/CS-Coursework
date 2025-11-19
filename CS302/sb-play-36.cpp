#include <iostream>       // Standard C++ library for input/output
#include <vector>         // Standard C++ library for dynamic arrays (vectors)
#include <unordered_map>  // Standard C++ library for hash tables (unordered maps)
#include "disjoint.h"     // Custom header file for disjoint set operations (used for grouping)

using namespace std;

// Superball class represents the game state and logic
class Superball {
  public:
    Superball(int argc, char **argv); // Constructor: Initializes the game using command-line inputs
    void play();                      // Function that decides the best move to make

    int r, c, mss, empty;  // r = rows, c = columns, mss = minimum score size, empty = count of empty spaces
    vector<int> board;     // A 1D array representing the board (stores characters as numbers)
    vector<int> goals;     // A 1D array indicating goal positions (cells that give points)
    vector<int> colors;    // A mapping of colors to numerical values
};

// Constructor: Reads game setup from command-line arguments
Superball::Superball(int argc, char **argv) {
    // Read board dimensions and minimum scoring size from command-line arguments
    r = stoi(argv[1]); // Convert first argument to integer (number of rows)
    c = stoi(argv[2]); // Convert second argument to integer (number of columns)
    mss = stoi(argv[3]); // Convert third argument to integer (minimum score size)
    
    // Resize `colors` vector to map ASCII characters (256 possible values)
    colors.resize(256, 0);

    // Process the fourth argument to assign numerical values to colors
    for (int i = 0; i < (int)strlen(argv[4]); i++) {
        colors[argv[4][i]] = 2 + i;         // Assign a unique number starting from 2
        colors[toupper(argv[4][i])] = 2 + i; // Ensure uppercase letters map to the same color
    }

    // Resize board and goals vectors to match board size
    board.resize(r * c);
    goals.resize(r * c, 0);
    empty = 0; // Counter for empty spaces

    // Read the board layout from standard input
    string s;
    for (int i = 0; i < r; i++) {
        cin >> s; // Read a row of board input as a string
        for (int j = 0; j < c; j++) {
            board[i * c + j] = s[j]; // Store board characters in a 1D array

            // Count empty cells ('.' and '*')
            if (board[i * c + j] == '.' || board[i * c + j] == '*') empty++;

            // Identify goal cells (uppercase letters or '*')
            if (isupper(board[i * c + j]) || board[i * c + j] == '*') {
                goals[i * c + j] = 1; // Mark this as a goal cell
                board[i * c + j] = tolower(board[i * c + j]); // Convert to lowercase for uniformity
            }
        }
    }
}

// Function that decides the best move to make
void Superball::play() {
    DisjointSetByRankWPC ds(r * c); // Create a disjoint set data structure for grouping connected cells

    // Step 1: Identify and merge connected cells of the same color
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            int idx = i * c + j; // Convert 2D index to 1D
            if (board[idx] == '.' || board[idx] == '*') continue; // Skip empty and goal cells

            // Check right neighbor: If it has the same color, merge the groups
            if (j + 1 < c && board[idx] == board[idx + 1]) {
                ds.Union(ds.Find(idx), ds.Find(idx + 1));
            }

            // Check bottom neighbor: If it has the same color, merge the groups
            if (i + 1 < r && board[idx] == board[idx + c]) {
                ds.Union(ds.Find(idx), ds.Find(idx + c));
            }
        }
    }

    // Step 2: Identify groups that can score
    unordered_map<int, int> set_size;             // Stores the size of each group (key: group leader)
    unordered_map<int, pair<int, int>> goal_cell; // Stores a goal cell for each group
    vector<pair<int, int>> scoring_moves;         // Stores possible scoring moves

    // Traverse the board and record group sizes and goal cell positions
    for (int i = 0; i < r * c; i++) {
        if (board[i] == '.' || board[i] == '*') continue; // Skip empty and goal cells

        int leader = ds.Find(i); // Find the representative (leader) of the group
        set_size[leader]++; // Increase count for this group

        // If this cell is a goal, store its position
        if (goals[i]) {
            goal_cell[leader] = {i / c, i % c};
        }
    }

// Step 3: Find valid scoring moves
for (unordered_map<int, int>::iterator it = set_size.begin(); it != set_size.end(); ++it) {
    int root = it->first;  // The representative of the group
    int group_size = it->second; // The size of the group
    
    if (group_size >= mss && goal_cell.count(root)) {
        scoring_moves.push_back(goal_cell[root]); // Store a goal cell if the group is large enough
    }
}


   // Step 4: Decide the best move to make
if (!scoring_moves.empty()) {
    // If there is a valid scoring move, execute it
    pair<int, int> first_move = scoring_moves.front();
    int row = first_move.first;
    int col = first_move.second;
    cout << "SCORE " << row << " " << col << "\n"; // Output the scoring move
} else {
    // If no scoring move is found, try swapping two different colored pieces
    for (int i = 0; i < r * c; i++) {
        if (board[i] != '.' && board[i] != '*') { // Ensure it's a valid piece
            for (int j = i + 1; j < r * c; j++) {
                if (board[j] != '.' && board[j] != '*' && board[i] != board[j]) {
                    cout << "SWAP " << i / c << " " << i % c << " " << j / c << " " << j % c << "\n";
                    return; // Perform the first valid swap and exit
                }
            }
        }
    }
}

}

// Main function: Initializes the game and starts playing
int main(int argc, char **argv) {
    Superball s(argc, argv); // Create a Superball instance with command-line arguments
    s.play(); // Call the function to make the best move
    return 0;
}
