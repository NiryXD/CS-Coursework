 /* Program Name: Bowling
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description: Basic bowling score tracker that the user can interact with*/

//  - gradescript command: `python3.11 scripts/test.py bowling.cpp`

#include <cmath> // All of the pound includes I know
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

const size_t NUMBER_OF_FRAMES = 10;
const size_t ROLLS_PER_FRAME = 2;
const size_t TOTAL_ROLLS = NUMBER_OF_FRAMES * 2 + 1;
const int NUMBER_OF_PINS = 10;

//Above are the provided variables, using constant so points aren't taken off

void printPlayerRolls(const vector<int> &playerRolls) {
    for (size_t i = 0; i < playerRolls.size(); ++i) {
        cerr << playerRolls[i] << " ";
    }
    cerr << endl;
}

//Above is meant to represent the rolls of the player via a vector.

vector<int> inputPlayerRolls() {
    vector<int> playerRolls(TOTAL_ROLLS, 0);
    int rollInput;

    for (size_t frame = 0; frame < NUMBER_OF_FRAMES; ++frame) {
        for (size_t roll = 0; roll < ROLLS_PER_FRAME; ++roll) {
            cout << "Enter score for frame " << frame + 1 << ", roll " << roll + 1 << ": ";
            cin >> rollInput;
            playerRolls[frame * 2 + roll] = rollInput;

            if (rollInput == NUMBER_OF_PINS && roll == 0) {
                break;
            }
        }
    }

    //Above promts the user to enter a score. Which inturn is stored into rollInput. rollInput is used later to calculate the player's
    //score

    if (playerRolls[TOTAL_ROLLS - 3] == NUMBER_OF_PINS) {
        cout << "Enter score for frame 10, roll 2: ";
        cin >> rollInput;
        playerRolls[TOTAL_ROLLS - 2] = rollInput;
        cout << "Enter score for frame 10, roll 3: ";
        cin >> rollInput;
        playerRolls[TOTAL_ROLLS - 1] = rollInput;
    }
    else if (playerRolls[TOTAL_ROLLS - 3] + playerRolls[TOTAL_ROLLS - 2] == NUMBER_OF_PINS) {
        cout << "Enter score for frame 10, roll 3: ";
        cin >> rollInput;
        playerRolls[TOTAL_ROLLS - 1] = rollInput;
    }
    return playerRolls;
}

// Above is the score limiter, can't have infinate plays!
// Below is a function mean to calculate the player's score

int calculateScore(const vector<int> &playerRolls) {
    int playerScore = 0;

    for (size_t frameNumber = 0; frameNumber < NUMBER_OF_FRAMES - 1; ++frameNumber) {
        const size_t currentFrame = frameNumber * 2;
        const size_t nextFrame = currentFrame + 2;
        const size_t nextNextFrame = nextFrame + 2;

        const int roll1 = playerRolls[currentFrame];
        const int roll2 = playerRolls[currentFrame + 1];

        const bool Strike = roll1 == NUMBER_OF_PINS;
        const bool Spare = roll1 + roll2 == NUMBER_OF_PINS;

        if (Strike == true && playerRolls[nextFrame] == NUMBER_OF_PINS) {
            playerScore += (NUMBER_OF_PINS + playerRolls[nextFrame] + +playerRolls[nextNextFrame]);
        }
        else if (Strike == true && !(playerRolls[nextFrame] == NUMBER_OF_PINS)) {
            playerScore += (NUMBER_OF_PINS + playerRolls[nextFrame] + playerRolls[nextFrame + 1]);
        }
        else if (Spare == true) {
            playerScore += (NUMBER_OF_PINS + playerRolls[nextFrame]);
        }
        else{
            playerScore += roll1 + roll2;
        }
    }


    if (playerRolls[16] == NUMBER_OF_PINS && playerRolls[18] == NUMBER_OF_PINS) {
        playerScore = playerScore - playerRolls[20];
        playerScore += playerRolls[19];
    }
    playerScore += playerRolls[18] + playerRolls[19] + playerRolls[20];
    return playerScore;
}

//Above calculates the score. This is done by utilizing a loop that checks out the vector. Recognizing whether the score merits a 
// strike, spare, or a numerical value. 


void gameSummary(const vector<string> &playerNames, const vector<int> &playerScores) {
    if (playerNames.empty()) {
        cout << "No players were entered." << endl;
        return;
    }

    //Just a error check

    cout << endl;

    int worstScoreIndex = 0;
    for (size_t i = 1; i < playerScores.size(); ++i) {
        if (playerScores[i] < playerScores[worstScoreIndex]) {
            worstScoreIndex = i;
        }
    }

    // To clown on the loser of the bunch

    int bestScoreIndex = 0;
    for (size_t i = 1; i < playerScores.size(); ++i) {
        if (playerScores[i] > playerScores[bestScoreIndex]) {
            bestScoreIndex = i;
        }
    }

    // Recognizing the winner to later reveal to the user of the program.

    for (size_t i = 0; i < playerNames.size(); ++i) {
        cout << playerNames[i] << " scored " << playerScores[i] << "." << endl;
    }

    cout << playerNames[worstScoreIndex] << " did the worst by scoring " << playerScores[worstScoreIndex] << "." << endl;

    cout << playerNames[bestScoreIndex] << " won the game by scoring " << playerScores[bestScoreIndex] << "." << endl;
}

// To top it all off, it just shows the user the scores and statistics of the game. Like who sucked and won.

int main() {
    vector<string> playerNames;
    vector<int> playerScores;

    while (true) {
        string inputString;

        cout << "Enter player's name (done for no more players): ";
        cin >> inputString;

        if (inputString == "done") {
            break;
        }

        playerNames.push_back(inputString);

        vector<int> playerRolls = inputPlayerRolls();
        int playerScore = calculateScore(playerRolls);

        playerScores.push_back(playerScore);

        if (!cin) {
            throw runtime_error("Invalid input");
        }
    }

    gameSummary(playerNames, playerScores);

    return 0;
}

// Classic return zero, and a error check before that
