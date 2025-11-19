// Program Name: Challenge 5
// Student Name: Ar-Raniry Ar-Rasyid
// Net ID: jzr266
// Student ID: 000-663-921
// Program Description: Program takes in the input of a matrix and outputs it as a MST

#include <bits/stdc++.h>

using namespace std;

const int bNum = 1e999999; // Big ahh number

// I refereced r/dailyprogrammer subreddit to see utilization of Prim's Algorithim
void prim(vector<vector<int>> &matrix, int nGraph)
{
    vector<int> parent(nGraph, -1);
    // Stores parent nodes
    vector<int> minimum(nGraph, bNum);
    // Stores smallest edge weight
    vector<bool> inMST(nGraph, false);
    // Store which nodes have been visited
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> priorityQ;

    minimum[0] = 0;
    // First node set to zero
    priorityQ.push({0, 0});
    // Throw it into the queue

    int tWeight = 0;
    // Int to store total weight

    while (!priorityQ.empty())
    {
        // Loop till empty
        int current = priorityQ.top().second;
        // Smallest node
        priorityQ.pop();
        // Remove it

        if (inMST[current])
            continue;
        // If node is already down, continue
        inMST[current] = true;
        // Mark it down if it ain't

        tWeight  = tWeight + minimum[current];
        // Add it to the total

        // Loop thru nodes
        for (int i = 0; i < nGraph; i++)
        {
            if (matrix[current][i] != -1 && !inMST[i] && matrix[current][i] < minimum[i])
            {
                // Check if neightbor aint in the MST
                // Also check if its the current smallest
                minimum[i] = matrix[current][i];
                // Update if it is the smallest
                parent[i] = current;
                priorityQ.push({minimum[i], i});
                // Add to queue
            }
        }
    }

    cout << tWeight << endl;

    vector<string> mstEdges;
    // Stores all edges

    // Start from second
    for (int i = 1; i < nGraph; i++)
    {
        if (parent[i] != -1)
        {
            char start = 'A' + parent[i];
            // Change into char
            char end = 'A' + i;
            string edge = "";
            edge = edge + min(start, end);
            edge = edge + max(start, end);
            mstEdges.push_back(edge);
        }
    }

    sort(mstEdges.begin(), mstEdges.end());
    // Sorts alphabetically

    for (int i = 0; i < mstEdges.size(); i++)
    {
        // Print edges
        cout << mstEdges[i] << endl;
    }
}

int main(int argc, char *argv[])
{
    int aNodes;
    // All the nodes
    bool firstCase = true;
    // For proper printing

    while (cin >> aNodes)
    {
        // Read until empty
        if (!firstCase)
        {
            cout << endl;
        }
        else
        {
            firstCase = false;
        }

        vector<vector<int>> matrix(aNodes, vector<int>(aNodes));
        // Matrix storing

        for (int i = 0; i < aNodes; i++)
        {
            for (int j = 0; j < aNodes; j++)
            {
                cin >> matrix[i][j];
            }
        }

        prim(matrix, aNodes);
    }

    return 0;
}