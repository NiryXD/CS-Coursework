
/* Program Name: Challenge 4
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description: This program recognizes what nodes reaches and doesn't reach other nodes.*/

#include <bits/stdc++.h>

using namespace std;

bool pathYes(const unordered_map<string, vector<string>> &graph, const string &start, const string &goTo)
{
    // Below, if start and end are the same return true
    if (start == goTo)
        return true;

    queue<string> queue;                 // Queue to see what to what is visited and not
    unordered_map<string, bool> visited; // Keeps track of visited nodes

    // Below checks for "start" in the graph, if it aint there return false
    if (graph.find(start) == graph.end())
        return false;

    // Below starts searching from start and then marking it as visited
    queue.push(start);
    visited[start] = true;

    while (!queue.empty())
    {
        // Keep going till it's empty (if not empty keep going)
        // Move on to next node, once done remove it
        string node = queue.front();
        queue.pop();

        // Just like before, if it aint exist STOP
        if (graph.find(node) != graph.end())
        {
            // Loop through the connected nodes
            const vector<string> &neighbors = graph.at(node); // Put the nodes inna vector

            for (size_t i = 0; i < neighbors.size(); i++)
            {
                string next = neighbors[i]; // Get the next node

                if (next == goTo)
                    return true; // if the next node exists then yay!
                if (!visited[next])
                {
                    visited[next] = true; // Mark it as "visited" and keep it pushing
                    queue.push(next);
                }
            }
        }
    }
    return false; // If the "if" statements didn't catch anything then return false
}

int main()
{
    string input; // Stuff given from the user
    int graphS = 0;

    while (getline(cin, input))
    {
        // Read all input and ++ the graph count
        graphS++;

        int edges = stoi(input);
        // convert input to int
        unordered_map<string, vector<string>> graph;

        for (int i = 0; i < edges; i++)
        // Run through the connections and store them
        {
            string start, goTo;
            cin >> start >> goTo;
            graph[start].push_back(goTo);
        }

        // Pathfinding, or rather confirmation
        int connections;
        cin >> connections;
        cin.ignore(); // Ignore the endl;

        if (graphS > 1)
            cout << endl;

        for (int i = 0; i < connections; i++)
        {
            string start, goTo;
            cin >> start >> goTo;

            // Printing out the results
            if (pathYes(graph, start, goTo))
            {
                cout << "In Graph " << graphS << " there is a path from " << start << " to " << goTo << endl;
            }
            else
            {
                cout << "In Graph " << graphS << " there is no path from " << start << " to " << goTo << endl;
            }
        }

        cin.ignore();
    }

    return 0;
}
