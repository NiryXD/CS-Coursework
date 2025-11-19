// Challenge 5: Minimum Spanning Tree
// Implements Prim's Algorithm to find the Minimum Spanning Tree (MST)
// Author: [Your Name]
// Date: [Current Date]

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>  // Needed for sorting

using namespace std;

const int INF = 1e9; // A large number representing "infinity"

// Function to find the Minimum Spanning Tree using Prim's Algorithm
void primMST(vector<vector<int>> &graph, int V) {
    vector<bool> inMST(V, false);   // Track visited vertices
    vector<int> parent(V, -1);      // Stores the MST edges
    vector<int> key(V, INF);        // Min edge weights to MST
    priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
    
    key[0] = 0; // Start with the first vertex
    pq.push({0, 0}); // {weight, vertex}
    
    int totalWeight = 0;
    
    while (!pq.empty()) {
        int u = pq.top().second;
        pq.pop();
        
        if (inMST[u]) continue;
        inMST[u] = true;
        
        totalWeight += key[u];
        
        // Traverse all neighbors
        for (int v = 0; v < V; v++) {
            if (graph[u][v] != -1 && !inMST[v] && graph[u][v] < key[v]) {
                key[v] = graph[u][v];
                parent[v] = u;
                pq.push({key[v], v});
            }
        }
    }
    
    // Output total weight
    cout << totalWeight << endl;
    
    // Collect MST edges
    vector<string> mstEdges;
    for (int i = 1; i < V; i++) {
        if (parent[i] != -1) {
            char src = 'A' + parent[i];
            char dest = 'A' + i;
            // Create edge with alphabetical order
            string edge = "";
            edge += min(src, dest);
            edge += max(src, dest);
            mstEdges.push_back(edge);
        }
    }
    
    // Sort edges lexicographically to match expected output
    sort(mstEdges.begin(), mstEdges.end());
    
    // Print sorted MST edges
    for (const string &edge : mstEdges) {
        cout << edge << endl;
    }
}

// Main Execution
int main(int argc, char *argv[]) {
    int V;
    bool firstCase = true;
    
    while (cin >> V) {
        // Add blank line between test cases (not before the first one)
        if (!firstCase) {
            cout << endl;
        } else {
            firstCase = false;
        }
        
        vector<vector<int>> graph(V, vector<int>(V));
        
        // Read adjacency matrix
        for (int i = 0; i < V; i++) {
            for (int j = 0; j < V; j++) {
                cin >> graph[i][j];
            }
        }
        
        primMST(graph, V);
    }
    
    return 0;
}