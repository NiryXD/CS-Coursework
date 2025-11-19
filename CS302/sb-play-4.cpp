// Program Name: Superball Play
// Student Name: Ar-Raniry Ar-Rasyid
// Net ID: jzr266
// Student ID: 000-663-921
// Program Description: Algorithm to get max points for Superball

#include <bits/stdc++.h>
#include "disjoint.h"

using namespace std;

class Superball {
public:
    Superball(int argc, char **argv);
    void make_move();
    
    int r, c, mss, empty;
    vector<int> board;
    vector<int> goals;
    vector<int> colors;

private:
    struct ScoringSet {
        int size;
        int value;
        int row;
        int col;
        char color;
    };
    
    int evaluate_swap(int r1, int c1, int r2, int c2);
    bool is_goal_adjacent(int row, int col);
};

void usage(const char *s) {
    cerr << "usage: sb-play rows cols min-score-size colors" << endl;
    if (s != NULL) cerr << s << endl;
    exit(1);
}

Superball::Superball(int argc, char **argv) {
    int i, j;
    string s;
    
    if (argc != 5) usage(NULL);
    
    if (sscanf(argv[1], "%d", &r) == 0 || r <= 0) usage("Bad rows");
    if (sscanf(argv[2], "%d", &c) == 0 || c <= 0) usage("Bad cols");
    if (sscanf(argv[3], "%d", &mss) == 0 || mss <= 0) usage("Bad min-score-size");
    
    colors.resize(256, 0);
    
    for (i = 0; i < (int) strlen(argv[4]); i++) {
        if (!isalpha(argv[4][i])) usage("Colors must be distinct letters");
        if (!islower(argv[4][i])) usage("Colors must be lowercase letters");
        if (colors[argv[4][i]] != 0) usage("Duplicate color");
        colors[argv[4][i]] = 2+i;
        colors[toupper(argv[4][i])] = 2+i;
    }
    
    board.resize(r*c);
    goals.resize(r*c, 0);
    empty = 0;
    
    for (i = 0; i < r; i++) {
        if (!(cin >> s)) {
            cerr << "Bad board: not enough rows on standard input" << endl;
            exit(1);
        }
        if ((int) s.size() != c) {
            cerr << "Bad board on row " << i << " - wrong number of characters." << endl;
            exit(1);
        }
        for (j = 0; j < c; j++) {
            if (s[j] != '*' && s[j] != '.' && colors[s[j]] == 0) {
                cerr << "Bad board row " << i << " - bad character " << s[j] << "." << endl;
                exit(1);
            }
            board[i*c+j] = s[j];
            if (board[i*c+j] == '.') empty++;
            if (board[i*c+j] == '*') empty++;
            if (isupper(board[i*c+j]) || board[i*c+j] == '*') {
                goals[i*c+j] = 1;
                board[i*c+j] = tolower(board[i*c+j]);
            }
        }
    }
}

bool Superball::is_goal_adjacent(int row, int col) {
    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};
    
    for (int i = 0; i < 4; i++) {
        int nr = row + dr[i];
        int nc = col + dc[i];
        
        if (nr >= 0 && nr < r && nc >= 0 && nc < c) {
            if (goals[nr * c + nc]) {
                return true;
            }
        }
    }
    return false;
}

int Superball::evaluate_swap(int r1, int c1, int r2, int c2) {
    int idx1 = r1 * c + c1;
    int idx2 = r2 * c + c2;
    
    if (board[idx1] == board[idx2]) {
        return 0;
    }
    
    swap(board[idx1], board[idx2]);
    
    int value = 0;
    const int dr[] = {-1, 1, 0, 0};
    const int dc[] = {0, 0, -1, 1};
    
    // Check both cells after swap
    for (int pos = 0; pos < 2; pos++) {
        int row = (pos == 0) ? r1 : r2;
        int col = (pos == 0) ? c1 : c2;
        int idx = row * c + col;
        char color = board[idx];
        
        int matching_neighbors = 0;
        bool near_goal = false;
        
        // Count matching neighbors and check for goals
        for (int d = 0; d < 4; d++) {
            int nr = row + dr[d];
            int nc = col + dc[d];
            
            if (nr >= 0 && nr < r && nc >= 0 && nc < c) {
                if (board[nr * c + nc] == color) {
                    matching_neighbors++;
                }
                if (goals[nr * c + nc]) {
                    near_goal = true;
                }
            }
        }
        
        // Calculate value based on color, matches, and goals
        if (matching_neighbors > 0) {
            int color_value = colors[color];
            int base_value = matching_neighbors * color_value;
            
            // Bonus for high-value colors
            if (color_value >= 5) {  // Red or Green
                base_value *= 2;
            } else if (color_value == 4) {  // Yellow
                base_value = base_value * 3 / 2;
            }
            
            // Bonus for goal positioning
            if (goals[idx]) {
                base_value *= 3;  // Triple value for goal cells
            } else if (near_goal) {
                base_value *= 2;  // Double value for cells adjacent to goals
            }
            
            value += base_value;
        }
    }
    
    swap(board[idx1], board[idx2]);
    return value;
}

void Superball::make_move() {
    // Create disjoint set
    DisjointSetByRankWPC ds(r * c);
    
    // Connect adjacent cells of the same color
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            int idx = i * c + j;
            if (board[idx] == '.' || board[idx] == '*') continue;
            
            // Check right and down
            if (j + 1 < c && board[idx] == board[idx + 1]) {
                ds.Union(ds.Find(idx), ds.Find(idx + 1));
            }
            if (i + 1 < r && board[idx] == board[(i+1) * c + j]) {
                ds.Union(ds.Find(idx), ds.Find((i+1) * c + j));
            }
        }
    }
    
    // Find and analyze connected components
    vector<int> set_sizes(r * c, 0);
    vector<int> set_values(r * c, 0);
    vector<bool> has_goal(r * c, false);
    vector<pair<int, int>> goal_pos(r * c, {-1, -1});
    vector<ScoringSet> scoring_sets;
    
    // Analyze board in one pass
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            int idx = i * c + j;
            if (board[idx] == '.' || board[idx] == '*') continue;
            
            int leader = ds.Find(idx);
            set_sizes[leader]++;
            set_values[leader] += colors[board[idx]];
            
            if (goals[idx]) {
                has_goal[leader] = true;
                goal_pos[leader] = {i, j};
            }
        }
    }
    
    // Find all scoring sets
    for (int i = 0; i < r * c; i++) {
        if (set_sizes[i] >= mss && has_goal[i]) {
            ScoringSet ss;
            ss.size = set_sizes[i];
            ss.value = set_values[i];
            ss.row = goal_pos[i].first;
            ss.col = goal_pos[i].second;
            ss.color = board[i];
            scoring_sets.push_back(ss);
        }
    }
    
    // Strategy 1: Score valuable sets
    if (!scoring_sets.empty()) {
        // Sort by value (highest first)
        sort(scoring_sets.begin(), scoring_sets.end(), 
            [](const ScoringSet& a, const ScoringSet& b) {
                return a.value > b.value;
            });
        
        ScoringSet best = scoring_sets[0];
        
        // Score in endgame or for large/valuable sets
        bool should_score = false;
        
        // Always score in endgame
        if (empty <= 20) {
            should_score = true;
        }
        // Score large sets
        else if (best.size >= mss + 2) {
            should_score = true;
        }
        // Score high-value sets
        else if (best.value >= 35) {
            should_score = true;
        }
        // Score red/green sets more aggressively
        else if (colors[best.color] >= 5 && best.size >= mss + 1) {
            should_score = true;
        }
        
        if (should_score) {
            cout << "SCORE " << best.row << " " << best.col << endl;
            return;
        }
    }
    
    // Strategy 2: Make the best swap
    
    // Find non-empty cells
    vector<pair<int, int>> cells;
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            int idx = i * c + j;
            if (board[idx] != '.' && board[idx] != '*') {
                cells.push_back({i, j});
            }
        }
    }
    
    if (cells.size() < 2) {
        cout << "SWAP 0 0 0 0" << endl;
        return;
    }
    
    // Prioritize high-value colors and goal-related cells
    vector<pair<int, int>> priority_cells;
    for (const pair<int, int>& pos : cells) {
        int i = pos.first;
        int j = pos.second;
        int idx = i * c + j;
        
        char cell_color = board[idx];
        int color_value = colors[cell_color];
        
        if (goals[idx] || is_goal_adjacent(i, j) || color_value >= 4) {
            priority_cells.push_back(pos);
        }
    }
    
    // Evaluate swaps
    int best_value = -1;
    int best_r1 = -1, best_c1 = -1, best_r2 = -1, best_c2 = -1;
    int swaps_checked = 0;
    
    // Check priority cell swaps first
    if (priority_cells.size() >= 2) {
        for (size_t i = 0; i < priority_cells.size(); i++) {
            for (size_t j = i+1; j < priority_cells.size(); j++) {
                swaps_checked++;
                
                int r1 = priority_cells[i].first;
                int c1 = priority_cells[i].second;
                int r2 = priority_cells[j].first;
                int c2 = priority_cells[j].second;
                
                int value = evaluate_swap(r1, c1, r2, c2);
                
                if (value > best_value) {
                    best_value = value;
                    best_r1 = r1;
                    best_c1 = c1;
                    best_r2 = r2;
                    best_c2 = c2;
                }
            }
        }
    }
    
    // If we didn't find a good swap with priority cells, try random ones
    if (best_value <= 5) {
        // Sort by color value
        sort(cells.begin(), cells.end(), 
            [this](const pair<int, int>& a, const pair<int, int>& b) {
                int idx_a = a.first * c + a.second;
                int idx_b = b.first * c + b.second;
                return colors[board[idx_a]] > colors[board[idx_b]];
            });
        
        // Random sampling of high-value cells
        srand(time(NULL));
        for (int k = 0; k < 30; k++) {
            int i = rand() % min(25, (int)cells.size());
            int j;
            do {
                j = rand() % min(25, (int)cells.size());
            } while (j == i);
            
            int r1 = cells[i].first;
            int c1 = cells[i].second;
            int r2 = cells[j].first;
            int c2 = cells[j].second;
            
            int value = evaluate_swap(r1, c1, r2, c2);
            
            if (value > best_value) {
                best_value = value;
                best_r1 = r1;
                best_c1 = c1;
                best_r2 = r2;
                best_c2 = c2;
            }
        }
    }
    
    // If we found a good swap, do it
    if (best_value > 0) {
        cout << "SWAP " << best_r1 << " " << best_c1 << " " 
             << best_r2 << " " << best_c2 << endl;
        return;
    }
    
    // If we have any scoring opportunity but no good swap, score it
    if (!scoring_sets.empty()) {
        cout << "SCORE " << scoring_sets[0].row << " " << scoring_sets[0].col << endl;
        return;
    }
    
    // Last resort: random swap
    srand(time(NULL));
    int idx1 = rand() % cells.size();
    int idx2;
    do {
        idx2 = rand() % cells.size();
    } while (idx2 == idx1 && cells.size() > 1);
    
    cout << "SWAP " << cells[idx1].first << " " << cells[idx1].second << " "
         << cells[idx2].first << " " << cells[idx2].second << endl;
}

int main(int argc, char **argv) {
    Superball s(argc, argv);
    s.make_move();
    return 0;
}