#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>
#include <unordered_map>

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
    fprintf(stderr, "usage: sb-analyze rows cols min-score-size colors\n");
    if (s != NULL)
        fprintf(stderr, "%s\n", s);
    exit(1);
}

Superball::Superball(int argc, char **argv)
{
    int i, j;
    string s;

    if (argc != 5)
        usage(NULL);

    if (sscanf(argv[1], "%d", &r) == 0 || r <= 0)
        usage("Bad rows");
    if (sscanf(argv[2], "%d", &c) == 0 || c <= 0)
        usage("Bad cols");
    if (sscanf(argv[3], "%d", &mss) == 0 || mss <= 0)
        usage("Bad min-score-size");

    colors.resize(256, 0);

    for (i = 0; i < (int)strlen(argv[4]); i++)
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
            fprintf(stderr, "Bad board: not enough rows on standard input\n");
            exit(1);
        }
        if ((int)s.size() != c)
        {
            fprintf(stderr, "Bad board on row %d - wrong number of characters.\n", i);
            exit(1);
        }
        for (j = 0; j < c; j++)
        {
            if (s[j] != '*' && s[j] != '.' && colors[s[j]] == 0)
            {
                fprintf(stderr, "Bad board row %d - bad character %c.\n", i, s[j]);
                exit(1);
            }
            board[i * c + j] = s[j];
            if (board[i * c + j] == '.')
                empty++;
            if (board[i * c + j] == '*')
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

    // Step 1: Merge connected same-color cells
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
                    ds.Union(root1, root2); // Ensure only leaders are united
            }

            if (i + 1 < r && board[idx] == board[idx + c])
            {
                int root1 = ds.Find(idx);
                int root2 = ds.Find(idx + c);
                if (root1 != root2)
                    ds.Union(root1, root2); // Ensure only leaders are united
            }
        }
    }

    // Step 2: Find scoring groups
    unordered_map<int, int> set_size;
    unordered_map<int, pair<int, int>> goal_cell;

    for (int i = 0; i < r * c; i++)
    {
        if (board[i] == '.' || board[i] == '*')
            continue;
        int leader = ds.Find(i);
        set_size[leader]++;

        if (goals[i])
        {
            goal_cell[leader] = {i / c, i % c}; // Store any valid goal cell
        }
    }

    // Step 3: Print valid scoring sets
    cout << "Scoring sets:\n";
    for (auto &entry : set_size)
    {
        int root = entry.first;
        if (entry.second >= mss && goal_cell.count(root))
        {
            int row = goal_cell[root].first, col = goal_cell[root].second;
            cout << "  Size: " << entry.second << "  Char: " << (char)board[root]
                 << "  Scoring Cell: " << row << "," << col << "\n";
        }
    }
}

int main(int argc, char **argv)
{
    Superball s(argc, argv);
    s.analyze();
    return 0;
}
