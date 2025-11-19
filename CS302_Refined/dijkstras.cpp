// Jonathan Lange
// ttb615
// Description: Program that finds the shortest path
// between two tiles on a grid, given tile costs
// dijsktras.cpp

#include <iostream>
#include <map>
#include <queue>
#include <algorithm>
using namespace std;

// point in grid
struct Point
{
    int row;
    int col;

    Point(int r = 0, int c = 0)
    {
        row = r;
        col = c;
    }
};

// node in Dijkstra's algorithm
struct Node
{
    Point point;
    int cost;

    Node(Point p, int c)
    {
        point = p;
        cost = c;
    }
};

// pick node with the lower cost, for priority queue
struct CompareNodes
{
    bool operator()(Node &a, Node &b)
    {
        return a.cost > b.cost;
    }
};

int main()
{
    // number of different tiles
    int numTiles;
    cin >> numTiles;

    // tile costs
    map<char, int> costs;
    for (int i = 0; i < numTiles; i++)
    {
        char name;
        int cost;
        cin >> name >> cost;
        costs[name] = cost;
    }

    // map dimensions
    int rows, cols;
    cin >> rows >> cols;

    // map layout
    vector<vector<char>> grid(rows, vector<char>(cols));
    for (int i = 0; i < rows; i++)
    {
        for (int j = 0; j < cols; j++)
        {
            cin >> grid[i][j];
        }
    }

    // start and end positions
    int startingRow, startingCol, endingRow, endingCol;
    cin >> startingRow >> startingCol >> endingRow >> endingCol;

    Point start(startingRow, startingCol);
    Point end(endingRow, endingCol);
    vector<vector<int>> distances(rows, vector<int>(cols, 99999999));       // unknown distances
    vector<vector<Point>> parent(rows, vector<Point>(cols, Point(-1, -1))); // parent pointers for path reconstruction
    priority_queue<Node, vector<Node>, CompareNodes> pq;                    // priority queue for Dijkstra's algorithm

    // add starting point
    distances[start.row][start.col] = 0;
    pq.push(Node(start, 0));

    // direction vectors
    int dr[] = {-1, 0, 1, 0};
    int dc[] = {0, 1, 0, -1};

    // Dijkstra's algorithm
    while (!pq.empty())
    {
        // get node with smallest cost
        Node current = pq.top();
        pq.pop();

        Point currentPoint = current.point;

        // look through neighbors (up, right, down, left)
        for (int i = 0; i < 4; i++)
        {
            int newRow = currentPoint.row + dr[i];
            int newCol = currentPoint.col + dc[i];

            // if neighbor is in grid...
            if (newRow >= 0 && newRow < rows && newCol >= 0 && newCol < cols)
            {
                int edgeCost = costs[grid[currentPoint.row][currentPoint.col]];         // find cost of leaving current tile
                int newCost = distances[currentPoint.row][currentPoint.col] + edgeCost; // new total cost

                // update, shorter path found
                if (newCost < distances[newRow][newCol])
                {
                    distances[newRow][newCol] = newCost;
                    parent[newRow][newCol] = currentPoint;
                    pq.push(Node(Point(newRow, newCol), newCost));
                }
            }
        }
    }

    vector<Point> path; // path to end
    Point current = end;

    // follow parents back to start and add them to path
    while (!(current.row == start.row && current.col == start.col))
    {
        path.push_back(current);
        current = parent[current.row][current.col];
    }

    path.push_back(start); // add start point

    // reverse (start to end)
    reverse(path.begin(), path.end());

    // output total cost and path
    cout << distances[end.row][end.col] << endl;
    for (vector<Point>::iterator it = path.begin(); it != path.end(); ++it)
    {
        cout << it->row << " " << it->col << endl;
    }

    return 0;
}
