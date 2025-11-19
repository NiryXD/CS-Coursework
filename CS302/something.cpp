#include <iostream>
#include <vector>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <fstream>
#include <sstream>

using namespace std;

const int INF = 1e9;

bool bfs(vector<vector<int>> &capacity, vector<int> &parent, int source, int sink) {
    int n = capacity.size();
    vector<bool> visited(n, false);
    queue<int> q;
    q.push(source);
    visited[source] = true;
    parent[source] = -1;

    while (!q.empty()) {
        int u = q.front(); q.pop();

        for (int v = 0; v < n; ++v) {
            if (!visited[v] && capacity[u][v] > 0) {
                q.push(v);
                parent[v] = u;
                visited[v] = true;
                if (v == sink) return true;
            }
        }
    }
    return false;
}

int edmonds_karp(vector<vector<int>> &capacity, int source, int sink, vector<int> &parent) {
    int max_flow = 0;
    while (bfs(capacity, parent, source, sink)) {
        int path_flow = INF;
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            path_flow = min(path_flow, capacity[u][v]);
        }
        for (int v = sink; v != source; v = parent[v]) {
            int u = parent[v];
            capacity[u][v] -= path_flow;
            capacity[v][u] += path_flow;
        }
        max_flow += path_flow;
    }
    return max_flow;
}

void solve_worddice(vector<unordered_set<char>> &dice, vector<string> &words) {
    int num_dice = dice.size();

    for (const string &word : words) {
        int num_letters = word.size();
        int source = 0, sink = num_dice + num_letters + 1;
        vector<vector<int>> capacity(sink + 1, vector<int>(sink + 1, 0));
        vector<int> parent(sink + 1, -1);

        for (int i = 1; i <= num_dice; ++i) capacity[source][i] = 1;
        map<int, int> letter_index;

        for (int i = 0; i < num_letters; ++i) {
            int letter_idx = num_dice + 1 + i;
            letter_index[letter_idx] = i;
            for (int j = 0; j < num_dice; ++j) {
                if (dice[j].count(word[i])) capacity[j + 1][letter_idx] = 1;
            }
            capacity[letter_idx][sink] = 1;
        }

        int max_flow = edmonds_karp(capacity, source, sink, parent);

        if (max_flow < num_letters) {
            cout << "Cannot spell " << word << endl;
            continue;
        }

        vector<int> dice_order(num_letters, -1);
        for (int d = 1; d <= num_dice; ++d) {
            for (int l = num_dice + 1; l < sink; ++l) {
                if (capacity[l][d] > 0) {
                    dice_order[letter_index[l]] = d - 1;
                }
            }
        }

        for (size_t i = 0; i < dice_order.size(); ++i) {
            if (i > 0) cout << ",";
            cout << dice_order[i];
        }
        cout << ": " << word << endl;
    }
}

void read_input(const string &dice_file, const string &words_file, vector<unordered_set<char>> &dice, vector<string> &words) {
    ifstream df(dice_file), wf(words_file);
    string line;

    while (getline(df, line)) {
        dice.push_back(unordered_set<char>(line.begin(), line.end()));
    }
    while (getline(wf, line)) {
        words.push_back(line);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cout << "Usage: ./worddice <dice_file> <words_file>" << endl;
        return 1;
    }
    vector<unordered_set<char>> dice;
    vector<string> words;
    read_input(argv[1], argv[2], dice, words);
    solve_worddice(dice, words);
    return 0;
}