#include <bits/stdc++.h> // Includes all standard libraries
#include "disjoint.h" // Includes the Disjoint Set data structure

using namespace std;

// Superball class to represent the game board and analyze its state
class Superball {
public:
    Superball(int argc, char **argv); // Constructor to initialize the board from input
    void analyze(); // Function to analyze the board state and find scoring opportunities

    int RENAME_rows, RENAME_cols, RENAME_minScoreSize, RENAME_emptyCells; // r: rows, c: columns, mss: min score size, empty: count of empty spaces
    vector<int> RENAME_boardGrid; // Stores the board representation
    vector<int> RENAME_goalPositions; // Stores the goal positions on the board
    vector<int> RENAME_colorMapping; // Maps each color character to a unique number
};

// Function to display usage instructions and exit if input is incorrect
void RENAME_showUsage(const char *s) {
    cout << "usage: sb-analyze rows cols min-score-size colors" << endl;
    if (s != NULL)
        cout << s << endl;
    exit(1);
}

// Constructor to initialize Superball object based on command-line arguments
Superball::Superball(int argc, char **argv) {
    int i, j;
    string RENAME_inputString;

    // Ensure correct number of arguments are provided
    if (argc != 5)
        RENAME_showUsage(NULL);

    // Convert input arguments to integers and validate them
    try {
        RENAME_rows = stoi(argv[1]); // Convert rows input to integer
        if (RENAME_rows <= 0) RENAME_showUsage("Bad rows");
    } catch (...) {
        RENAME_showUsage("Bad rows");
    }

    try {
        RENAME_cols = stoi(argv[2]); // Convert columns input to integer
        if (RENAME_cols <= 0) RENAME_showUsage("Bad cols");
    } catch (...) {
        RENAME_showUsage("Bad cols");
    }

    try {
        RENAME_minScoreSize = stoi(argv[3]); // Convert minimum score size input to integer
        if (RENAME_minScoreSize <= 0) RENAME_showUsage("Bad min-score-size");
    } catch (...) {
        RENAME_showUsage("Bad min-score-size");
    }

    // Initialize colors array with 256 entries (ASCII characters)
    RENAME_colorMapping.resize(256, 0);

    // Parse the color letters from the input
    for (i = 0; i < static_cast<int>(strlen(argv[4])); i++) {
        if (!isalpha(argv[4][i])) // Ensure the color is a letter
            RENAME_showUsage("Colors must be distinct letters");
        if (!islower(argv[4][i])) // Ensure the color is lowercase
            RENAME_showUsage("Colors must be lowercase letters");
        if (RENAME_colorMapping[argv[4][i]] != 0) // Check for duplicate colors
            RENAME_showUsage("Duplicate color");
        
        // Assign a unique number to each color (starting from 2)
        RENAME_colorMapping[argv[4][i]] = 2 + i;
        RENAME_colorMapping[toupper(argv[4][i])] = 2 + i; // Support uppercase letters
    }

    // Initialize board and goal positions
    RENAME_boardGrid.resize(RENAME_rows * RENAME_cols);
    RENAME_goalPositions.resize(RENAME_rows * RENAME_cols, 0);
    RENAME_emptyCells = 0;

    // Read the board state from standard input
    for (i = 0; i < RENAME_rows; i++) {
        if (!(cin >> RENAME_inputString)) {
            cout << "Bad board: not enough rows on standard input" << endl;
            exit(1);
        }
        if (static_cast<int>(RENAME_inputString.size()) != RENAME_cols) {
            cout << "Bad board on row " << i << " - wrong number of characters." << endl;
            exit(1);
        }
        for (j = 0; j < RENAME_cols; j++) {
            // Validate board characters
            if (RENAME_inputString[j] != '*' && RENAME_inputString[j] != '.' && RENAME_colorMapping[RENAME_inputString[j]] == 0) {
                cout << "Bad board row " << i << " - bad character " << RENAME_inputString[j] << "." << endl;
                exit(1);
            }
            
            // Store board character
            RENAME_boardGrid[i * RENAME_cols + j] = RENAME_inputString[j];
            
            // Count empty cells ('.' or '*')
            if (RENAME_boardGrid[i * RENAME_cols + j] == '.' || RENAME_boardGrid[i * RENAME_cols + j] == '*')
                RENAME_emptyCells++;
            
            // Mark goal positions (uppercase letters and '*')
            if (isupper(RENAME_boardGrid[i * RENAME_cols + j]) || RENAME_boardGrid[i * RENAME_cols + j] == '*') {
                RENAME_goalPositions[i * RENAME_cols + j] = 1;
                RENAME_boardGrid[i * RENAME_cols + j] = tolower(RENAME_boardGrid[i * RENAME_cols + j]); // Convert to lowercase for processing
            }
        }
    }
}

// Function to analyze the board and identify scoring sets
void Superball::analyze() {
    DisjointSetByRankWPC RENAME_disjointSet(RENAME_rows * RENAME_cols); // Initialize Disjoint Set for tracking connected components
    
    // Iterate over the board to merge adjacent cells with the same color
    for (int i = 0; i < RENAME_rows; i++) {
        for (int j = 0; j < RENAME_cols; j++) {
            int RENAME_index = i * RENAME_cols + j; // Get 1D index from 2D coordinates
            if (RENAME_boardGrid[RENAME_index] == '.' || RENAME_boardGrid[RENAME_index] == '*')
                continue;

            // Check right neighbor and merge sets if colors match
            if (j + 1 < RENAME_cols && RENAME_boardGrid[RENAME_index] == RENAME_boardGrid[RENAME_index + 1]) {
                int RENAME_rootA = RENAME_disjointSet.Find(RENAME_index);
                int RENAME_rootB = RENAME_disjointSet.Find(RENAME_index + 1);
                if (RENAME_rootA != RENAME_rootB)
                    RENAME_disjointSet.Union(RENAME_rootA, RENAME_rootB);
            }
        }
    }
}
