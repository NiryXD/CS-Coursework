#include <iostream>
#include <unordered_map>
#include <vector>
#include <queue>
#include <sstream>

using namespace std;

// Function to perform BFS to check if a path exists
bool bfsPathExists(const unordered_map<string, vector<string>>& graph, const string& src, const string& dst) {
    if (src == dst) return true;

    queue<string> q;
    unordered_map<string, bool> visited;

    // Check if the source node exists in the graph
    if (graph.find(src) == graph.end()) return false;

    q.push(src);
    visited[src] = true;

    while (!q.empty()) {
        string node = q.front();
        q.pop();

        // Ensure node exists in graph before accessing it
        if (graph.find(node) != graph.end()) {
            for (const string& neighbor : graph.at(node)) {
                if (neighbor == dst) return true;
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }
    }
    return false;
}

int main() {
    string line;
    int graphNum = 0;

    while (getline(cin, line)) {
        graphNum++;

        // Read number of edges
        int nEdges = stoi(line);
        unordered_map<string, vector<string>> graph;

        // Read edge list
        for (int i = 0; i < nEdges; i++) {
            string src, dst;
            cin >> src >> dst;
            graph[src].push_back(dst); // Directed edge from src -> dst
        }

        // Read number of paths to check
        int nPaths;
        cin >> nPaths;
        cin.ignore();  // Ignore the newline after nPaths

        if (graphNum > 1) cout << endl; // Separate output for different graphs

        // Process each path query
        for (int i = 0; i < nPaths; i++) {
            string src, dst;
            cin >> src >> dst;

            if (bfsPathExists(graph, src, dst)) {
                cout << "In Graph " << graphNum << " there is a path from " << src << " to " << dst << endl;
            } else {
                cout << "In Graph " << graphNum << " there is no path from " << src << " to " << dst << endl;
            }
        }

        // Clear the input buffer
        cin.ignore();
    }

    return 0;
}
