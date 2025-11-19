#include <bits/stdc++.h>
#include "disjoint.h"

using namespace std;

class Superball
{
public:
    Superball(int argc, char **argv);
    void analyze();

    int r, c, mss, empty;
    vector<int> board;
    vector<int> goals;
    vector<int> colors;
};

void usage(const char *s)
{
    cout << "usage: sb-analyze rows cols min-score-size colors" << endl;
    if (s != NULL)
        cout << s << endl;
    exit(1);
}

Superball::Superball(int argc, char **argv)
{
    int i, j;
    string s;

    if (argc != 5)
        usage(NULL);

    try {
        r = stoi(argv[1]);
        if (r <= 0) usage("Bad rows");
    } catch (...) {
        usage("Bad rows");
    }

    try {
        c = stoi(argv[2]);
        if (c <= 0) usage("Bad cols");
    } catch (...) {
        usage("Bad cols");
    }

    try {
        mss = stoi(argv[3]);
        if (mss <= 0) usage("Bad min-score-size");
    } catch (...) {
        usage("Bad min-score-size");
    }

    colors.resize(256, 0);

    for (i = 0; i < static_cast<int>(strlen(argv[4])); i++)
    {
        if (!isalpha(argv[4][i]))
            usage("Colors must be distinct letters");
        if (!islower(argv[4][i]))
            usage("Colors must be lowercase letters");
        if (colors[argv[4][i]] != 0)
            usage("Duplicate color");
        colors[argv[4][i]] = 2 + i;
        colors[toupper(argv[4][i])] = 2 + i;
    }

    board.resize(r * c);
    goals.resize(r * c, 0);
    empty = 0;

    for (i = 0; i < r; i++)
    {
        if (!(cin >> s))
        {
            cout << "Bad board: not enough rows on standard input" << endl;
            exit(1);
        }
        if (static_cast<int>(s.size()) != c)
        {
            cout << "Bad board on row " << i << " - wrong number of characters." << endl;
            exit(1);
        }
        for (j = 0; j < c; j++)
        {
            if (s[j] != '*' && s[j] != '.' && colors[s[j]] == 0)
            {
                cout << "Bad board row " << i << " - bad character " << s[j] << "." << endl;
                exit(1);
            }
            board[i * c + j] = s[j];
            if (board[i * c + j] == '.' || board[i * c + j] == '*')
                empty++;
            if (isupper(board[i * c + j]) || board[i * c + j] == '*')
            {
                goals[i * c + j] = 1;
                board[i * c + j] = tolower(board[i * c + j]);
            }
        }
    }
}

void Superball::analyze()
{
    DisjointSetByRankWPC ds(r * c);
    
    for (int i = 0; i < r; i++)
    {
        for (int j = 0; j < c; j++)
        {
            int idx = i * c + j;
            if (board[idx] == '.' || board[idx] == '*')
                continue;

            if (j + 1 < c && board[idx] == board[idx + 1])
            {
                int root1 = ds.Find(idx);
                int root2 = ds.Find(idx + 1);
                if (root1 != root2)
                    ds.Union(root1, root2);
            }

            if (i + 1 < r && board[idx] == board[idx + c])
            {
                int root1 = ds.Find(idx);
                int root2 = ds.Find(idx + c);
                if (root1 != root2)
                    ds.Union(root1, root2);
            }
        }
    }

    unordered_map<int, int> set_size;
    unordered_map<int, pair<int, int>> goal_cell;

    for (int i = 0; i < r * c; i++)
    {
        if (board[i] == '.' || board[i] == '*')
            continue;
        int leader = ds.Find(i);
        set_size[leader] = set_size[leader] + 1;

        if (goals[i])
        {
            goal_cell[leader] = make_pair(i / c, i % c);
        }
    }

    cout << "Scoring sets:" << endl;
    for (unordered_map<int, int>::iterator it = set_size.begin(); it != set_size.end(); ++it)
    {
        int root = it->first;
        if (it->second >= mss && goal_cell.find(root) != goal_cell.end())
        {
            int row = goal_cell[root].first;
            int col = goal_cell[root].second;
            cout << "  Size: " << it->second << "  Char: " << static_cast<char>(board[root])
                 << "  Scoring Cell: " << row << "," << col << endl;
        }
    }
}

int main(int argc, char **argv)
{
    Superball s(argc, argv);
    s.analyze();
    return 0;
}
