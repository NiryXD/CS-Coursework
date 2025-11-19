#include <bits/stdc++.h>
#include "disjoint.h"

using namespace std;

class Superball {
  public:
    Superball(int argc, char **argv);
    void play();

    int r, c, mss, empty;
    vector<int> board;
    vector<int> goals;
    vector<int> colors;
};

Superball::Superball(int argc, char **argv) {
    r = stoi(argv[1]);
    c = stoi(argv[2]);
    mss = stoi(argv[3]);
    
    colors.resize(256, 0);
    for (int i = 0; i < (int)strlen(argv[4]); i++) {
        colors[argv[4][i]] = 2 + i;
        colors[toupper(argv[4][i])] = 2 + i;
    }

    board.resize(r * c);
    goals.resize(r * c, 0);
    empty = 0;
    
    string s;
    for (int i = 0; i < r; i++) {
        cin >> s;
        for (int j = 0; j < c; j++) {
            board[i * c + j] = s[j];
            if (board[i * c + j] == '.' || board[i * c + j] == '*') empty++;
            if (isupper(board[i * c + j]) || board[i * c + j] == '*') {
                goals[i * c + j] = 1;
                board[i * c + j] = tolower(board[i * c + j]);
            }
        }
    }
}

void Superball::play() {
    DisjointSetByRankWPC ds(r * c);

    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            int idx = i * c + j;
            if (board[idx] == '.' || board[idx] == '*') continue;
            if (j + 1 < c && board[idx] == board[idx + 1]) {
                ds.Union(ds.Find(idx), ds.Find(idx + 1));
            }
            if (i + 1 < r && board[idx] == board[idx + c]) {
                ds.Union(ds.Find(idx), ds.Find(idx + c));
            }
        }
    }

    unordered_map<int, int> set_size;
    unordered_map<int, pair<int, int>> goal_cell;
    vector<pair<int, int>> scoring_moves;

    for (int i = 0; i < r * c; i++) {
        if (board[i] == '.' || board[i] == '*') continue;
        int leader = ds.Find(i);
        set_size[leader]++;
        if (goals[i]) {
            goal_cell[leader] = {i / c, i % c};
        }
    }

    for (auto &entry : set_size) {
        int root = entry.first;
        if (entry.second >= mss && goal_cell.count(root)) {
            scoring_moves.push_back(goal_cell[root]);
        }
    }

    if (!scoring_moves.empty()) {
        auto [row, col] = scoring_moves.front();
        cout << "SCORE " << row << " " << col << "\n";
    } else {
        for (int i = 0; i < r * c; i++) {
            if (board[i] != '.' && board[i] != '*') {
                for (int j = i + 1; j < r * c; j++) {
                    if (board[j] != '.' && board[j] != '*' && board[i] != board[j]) {
                        cout << "SWAP " << i / c << " " << i % c << " " << j / c << " " << j % c << "\n";
                        return;
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv) {
    Superball s(argc, argv);
    s.play();
    return 0;
}
