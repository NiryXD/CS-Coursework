/*
 * Program Name: Project 5
 * Student Names: Jonathan Lange, Ar-Raniry Ar-Rasyid
 * Net ID: ttb615, jzr266
 * Student ID: 000-693-721, 000-663-921
 * Program Description: Program that checks if words can be spelled using a set of dice by using the Edmonds-Karp algorithm.
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <string>   

using namespace std;

struct Edge;

// node in graph
struct Node
{
    int ID;               // identifier for node (unique for each one)
    string label;         // shows what kind of node this node is
    vector<Edge *> edges; // edges from node
    Edge *backEdge;       // edge used to reach node in BFS
    bool visited;

    // constructor
    Node(int ID, const string &label)
    {
        this->ID = ID;
        this->label = label;
        this->backEdge = NULL;
        this->visited = false;
    }
};

// edge in graph
struct Edge
{
    Node *from;    // source node
    Node *to;      // destination node
    int capacity;  // how much more flow can go through edge
    Edge *reverse; // reverse of current edge

    // constructor
    Edge(Node *from, Node *to, int capacity)
    {
        this->from = from;
        this->to = to;
        this->capacity = capacity;
        this->reverse = NULL;
    }
};

// graph for dice-letter connections
class Graph
{
public:
    vector<Node *> nodes; // all nodes in graph
    Node *source;         // source node
    Node *sink;           // sink node
    size_t diceEnd;       // end of dice nodes

    // make graph with nodes
    Graph(size_t numDice)
    {
        source = new Node(0, "SOURCE");
        nodes.push_back(source);
        for (size_t i = 0; i < numDice; ++i)
        {
            int ID = i + 1;
            nodes.push_back(new Node(ID, "DIE"));
        }
        diceEnd = nodes.size(); // save position after dice nodes
    }

    // reset graph
    void reset()
    {
        while (nodes.size() > diceEnd)
        {
            delete nodes.back();
            nodes.pop_back();
        }
    }

    // adds directed edge
    void addEdge(Node *fromNode, Node *toNode, int cap = 1)
    {
        Edge *forwardEdge = new Edge(fromNode, toNode, cap);
        Edge *reverseEdge = new Edge(toNode, fromNode, 0);
        forwardEdge->reverse = reverseEdge;
        reverseEdge->reverse = forwardEdge;
        fromNode->edges.push_back(forwardEdge);
        toNode->edges.push_back(reverseEdge);
    }

    // find path from source to sink
    bool bfs()
    {
        // reset visited flags and back edges
        for (vector<Node *>::iterator it = nodes.begin(); it != nodes.end(); ++it)
        {
            Node *n = *it;
            n->visited = false;
            n->backEdge = NULL;
        }

        queue<Node *> nodeQueue;
        nodeQueue.push(source);
        source->visited = true;

        // breadth-first search
        while (!nodeQueue.empty())
        {
            Node *fromNode = nodeQueue.front();
            nodeQueue.pop();

            for (vector<Edge *>::iterator it = fromNode->edges.begin(); it != fromNode->edges.end(); ++it)
            {
                Edge *currentEdge = *it;
                if (currentEdge->capacity > 0 && !currentEdge->to->visited)
                {
                    currentEdge->to->visited = true;
                    currentEdge->to->backEdge = currentEdge;
                    if (currentEdge->to == sink)
                    {
                        return true; // found path
                    }
                    nodeQueue.push(currentEdge->to);
                }
            }
        }
        return false; // no path found
    }

    // sees if a word can be spelled using dice
    bool canSpell(string &word, vector<string> &dice, vector<int> &result)
    {
        reset();

        vector<Node *> letterNodes;

        // node for all letters in word
        for (size_t i = 0; i < word.size(); ++i)
        {
            Node *ln = new Node(nodes.size(), string(1, word[i]));
            nodes.push_back(ln);
            letterNodes.push_back(ln);
        }

        // sink node
        sink = new Node(nodes.size(), "SINK");
        nodes.push_back(sink);

        // connect source to dice nodes
        for (size_t i = 1; i <= dice.size(); ++i)
        {
            addEdge(source, nodes[i]);
        }

        // connect dice nodes to matching letter nodes
        for (size_t i = 0; i < dice.size(); ++i)
        {
            for (size_t j = 0; j < word.size(); ++j)
            {
                if (dice[i].find(word[j]) != string::npos)
                {
                    addEdge(nodes[i + 1], letterNodes[j]);
                }
            }
        }

        // connect letter nodes to sink
        for (size_t i = 0; i < letterNodes.size(); ++i)
        {
            addEdge(letterNodes[i], sink);
        }

        // Edmonds-Karp to find maximum matching
        int flow = 0;
        while (bfs())
        {
            // go back to source and change capacities
            Node *currentNode = sink;
            while (currentNode != source)
            {
                Edge *currentEdge = currentNode->backEdge;
                currentEdge->capacity -= 1;
                currentEdge->reverse->capacity += 1;
                currentNode = currentEdge->from;
            }
            ++flow;
        }

        // can't spell if flow is not equal to word length
        if (flow != (int)word.size())
        {
            return false;
        }

        // see which dice were used for each letter
        result.clear();
        for (size_t j = 0; j < word.size(); ++j)
        {
            Node *ln = nodes[diceEnd + j];
            for (size_t i = 0; i < ln->edges.size(); ++i)
            {
                Edge *currentEdge = ln->edges[i];
                if (currentEdge->to->label == "DIE" && currentEdge->capacity > 0)
                {
                    result.push_back(currentEdge->to->ID - 1); // dice index
                    break;
                }
            }
        }

        return true;
    }
};

int main(int argc, char *argv[])
{
    ifstream diceFile(argv[1]), wordFile(argv[2]);
    vector<string> dice, words;

    // read dice from file
    for (string line; getline(diceFile, line);)
    {
        if (!line.empty())
        {
            dice.push_back(line);
        }
    }

    // read words from file
    for (string line; getline(wordFile, line);)
    {
        if (!line.empty())
        {
            words.push_back(line);
        }
    }

    // for all words, check if it can be spelled with dice
    for (size_t i = 0; i < words.size(); ++i)
    {
        string &w = words[i];
        Graph g(dice.size());
        vector<int> IDs;
        if (g.canSpell(w, dice, IDs))
        {
            // print indices of dice used
            for (size_t j = 0; j < IDs.size(); ++j)
            {
                cout << IDs[j];
                if (j + 1 < IDs.size())
                {
                    cout << ',';
                }
            }
            cout << ": " << w << "\n";
        }
        else
        {
            cout << "Cannot spell " << w << "\n";
        }
    }

    return 0;
}
