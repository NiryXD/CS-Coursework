#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

struct Room {
    int north = -1; // -1 means no room
    int south = -1;
    int east = -1;
    int west = -1;
    string name;
    string description;
};

// Take in a direction char, return the room index in that direction.
int getExit(const Room &room, const char direction) {
    switch (direction) {
        case 'n':
            return room.north;
        case 's':
            return room.south;
        case 'e':
            return room.east;
        case 'w':
            return room.west;
        default:
            return -1; // Invalid direction
    }
}

// Take in a direction char and index and assign the exit to the room.
void setExit(Room &room, const char direction, const int roomIndex) {
    switch (direction) {
        case 'n':
            room.north = roomIndex;
            break;
        case 's':
            room.south = roomIndex;
            break;
        case 'e':
            room.east = roomIndex;
            break;
        case 'w':
            room.west = roomIndex;
            break;
        default:
            cerr << "Error: Invalid direction" << endl;
            break;
    }
}

// Take in a name and description and assign them to the room.
void setInfo(Room &room, const string &name, const string &description) {
    room.name = name;
    room.description = description;
}

// Print the room's name, description, and exits.
void look(const Room &room) {
    cout << room.name << endl;
    cout << room.description << endl;
    cout << "Exits:";
    if (room.north != -1) cout << " n";
    if (room.south != -1) cout << " s";
    if (room.east != -1) cout << " e";
    if (room.west != -1) cout << " w";
    cout << endl;
}

// For debugging
void dumpRooms(const Room *const rooms, const size_t roomCount) {
    for (size_t i = 0; i < roomCount; ++i) {
        cout << "Room " << i << endl;
        cout << "  name: " << rooms[i].name << endl;
        cout << "  description: " << rooms[i].description << endl;
        cout << "  north: " << rooms[i].north << endl;
        cout << "  south: " << rooms[i].south << endl;
        cout << "  east:  " << rooms[i].east << endl;
        cout << "  west:  " << rooms[i].west << endl;
    }
}

// Removes whitespace inplace from the front and back of a string.
// "\n hello\n \n" -> "hello"
void stripWhitespace(string &str) {
    while (!str.empty() && isspace(str.back())) {
        str.pop_back();
    }
    while (!str.empty() && isspace(str.front())) {
        str.erase(str.begin());
    }
}

// Take in a char 'n', 's', 'e', or 'w' and return the full direction name.
// e.g. 'n' -> "NORTH"
// Helpful for converting user input to direction names.
string getDirectionName(const char direction) {
    switch (direction) {
        case 'n':
            return "NORTH";
        case 's':
            return "SOUTH";
        case 'e':
            return "EAST";
        case 'w':
            return "WEST";
        default:
            return "";
    }
}

const Room *loadRooms(const string dungeonFilename) {
    ifstream fin(dungeonFilename);
    if (!fin.is_open()) {
        cerr << "Error: failed to open file" << endl;
        exit(1); // Exit the program if file failed to open
    }

    // Count the number of tildes in the file to determine the number of rooms
    size_t tildeCount = 0;
    string line;
    while (getline(fin, line)) {
        if (line == "~") {
            tildeCount++;
        }
    }
    size_t roomCount = tildeCount / 3; // Each room has 3 tildes

    // Allocate memory for rooms
    Room *rooms = new Room[roomCount];

    // Jump back to the beginning of the file
    fin.clear();
    fin.seekg(0);

    // Read through the file again to read in the rooms
    for (size_t i = 0; i < roomCount; ++i) {
        getline(fin, rooms[i].name);
        getline(fin, rooms[i].description);

        string exitsString;
        getline(fin, exitsString);
        istringstream exitsStream(exitsString);
        char direction;
        int roomIndex;
        while (exitsStream >> direction >> roomIndex) {
            setExit(rooms[i], direction, roomIndex);
        }
    }

    fin.close();
    return rooms;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        cerr << "Usage: ./mud filename" << endl;
        return 1; // Return a non-zero value to indicate an error
    }

    // Load rooms from the file into memory
    const Room *rooms = loadRooms(argv[1]);
    if (rooms == nullptr) {
        cerr << "Error: failed to open file" << endl;
        return 1; // Return a non-zero value to indicate an error
    }

    // Print room details
    dumpRooms(rooms, 10); // Assuming there are 10 rooms

    // Cleanup: Deallocate memory
    delete[] rooms;

    return 0;
}
