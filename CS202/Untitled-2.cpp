/* Program Name: SS Solver
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: Does placement on binary grid using recursion*/

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstring>

using namespace std;

class Shape {
public:
    vector<string> grid; // initalize vector for grid
    size_t rows; // initalize rows
    size_t cols; // initalize columns
    
    // set all to zero
    Shape() : rows(0), cols(0) {}
    
    // intializes the shape
    void initialize(const vector<string>& g) {
        grid = g;
        rows = grid.size();
        cols = grid.empty() ? 0 : grid[0].size();
    }
    
    // sees if the shape can work with the rows and columns we have avaiable 
    bool canApplyAt(const Shape& target, size_t row, size_t col) const {
        if (row + rows > target.rows || 
            col + cols > target.cols) {
            return false;
        }
        return true;
    }
    
    // puts the shape in the grid
    Shape applyAt(int row, int col) const {
    Shape result = *this;
    for (size_t i = 0; i < rows; i++)
        for (size_t j = 0; j < cols; j++)
    // I had a bunch of if statements hear, and its neat to use ternary operators
            result.grid[row + i][col + j] = grid[i][j] == '1' ? 
                (result.grid[row + i][col + j] == '1' ? '0' : '1') : 
                result.grid[row + i][col + j];
    return result;
}
    
    // checks to see if all cells are 1
    bool Is_Complete() const {
        for (const string& row : grid) {
            for (char c : row) {
                if (c != '1') return false;
            }
        }
        return true;
    }
};

// tries to solve the shape
bool solve(const Shape& current, vector<Shape>& shapes, vector<string>& answer, int shapeIndex) {
    // checks if grid is done
    if (shapeIndex >= (int)shapes.size()) {
        return current.Is_Complete();
    }
    
    const Shape& shape = shapes[shapeIndex];
    
    // try do to the current shape any way it can
    for (int row = 0; row <= (int)current.rows - (int)shape.rows; row++) {
        for (int col = 0; col <= (int)current.cols - (int)shape.cols; col++) {
            if (shape.canApplyAt(current, row, col)) {
                Shape next = current;
                // flips the 1s and zeros to apply the shape
                for (size_t i = 0; i < shape.rows; i++) {
                    for (size_t j = 0; j < shape.cols; j++) {
                        if (shape.grid[i][j] == '1') {
                            next.grid[row + i][col + j] = 
                                (next.grid[row + i][col + j] == '1') ? '0' : '1';
                        }
                    }
                }
                
                // formatting the movement
                string shift;
                for (const string& s : shape.grid) {
                    shift += s + " ";
                }
                shift += to_string(row) + " " + to_string(col);
                
                answer.push_back(shift);
                // try to solve the next one
                if (solve(next, shapes, answer, shapeIndex + 1)) {
                    return true;
                }
                answer.pop_back(); // go back if nothing
            }
        }
    }
    
    return false;
}

vector<Shape> breakShapes(istream& in) {
    vector<Shape> shapes;
    string line;
    
    // read each line
    while (getline(in, line)) {
        istringstream iss(line);
        string part;
        vector<string> currentShape;
        size_t maxLen = 0;
        
        // divide it into parts
        while (iss >> part) {
            if (part.find_first_not_of("01") == string::npos) {
                currentShape.push_back(part);
                maxLen = max(maxLen, part.length());
            }
        }
        
        // if the shape is good, intialize and store
        if (!currentShape.empty()) {
            Shape shape;
            shape.initialize(currentShape);
            shapes.push_back(shape);
        }
    }
    
    return shapes;
}

int main(int argc, char *argv[]) {
    if (argc < 2) return 0;
    
    // initalize the grid
    Shape initial;
    vector<string> grid;
    size_t cols = strlen(argv[1]);
    // so strlen basically checks out the length
    
    for (int diddy = 1; diddy < argc; diddy++) {
        if (strlen(argv[diddy]) != cols) return 0; // make sure all rows are the same length
        string row = argv[diddy];
        for (char party : row) {
            if (party != '0' && party != '1') return 0; // make sure its just ones and zeros
        }
        grid.push_back(row);
    }
    initial.initialize(grid);
    
    // read and parse the shapes
    vector<Shape> shapes = breakShapes
(cin);
    
    // find the answer
    vector<string> answer;
    if (solve(initial, shapes, answer, 0)) {
        // cout the answer
        for (const string& shift : answer) {
            cout << shift << endl;
        }
    }
    
    return 0;
}
