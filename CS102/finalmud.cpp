/* Program Name: Multi-User Dungeons
 * Student Name: Ar-Raniry Ar-Rasyid
 * Net ID: jzr266
 * Student ID: 000-663-921
 * Program Description: An interactable text adventure functioning through the use of pointers */

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

struct Room {
    int north = -1;
    int south = -1;
    int east = -1;
    int west = -1;
    string name;
    string description;
};

int getExit(const Room &room, const char direction) {
    switch (direction)
    {
        case 'n':
        return room.north;
        case 's':
        return room.south;
        case 'e':
        return room.east;
        case 'w':
        return room.west;
        default:
        return -1;

// I utilize a switch statement to take in user input in order to return a corresponding room index.

    }
}

void setExit(Room &room, const char direction, const int roomIndex) {
        
        switch (direction)
        {
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
        cerr << "Error" << endl;
        break;



// I used the same line of reasoning here, just for the exits.

        }
}

void setInfo(Room &room, const string &name, const string &description) {
    room.name = name;
    room.description = description;

// 

}

void look(const Room &room) {
cout << room.name << endl;
cout << room.description << endl;
cout << "Exits:"; 
if (room.north != -1)
cout << "n";
if (room.south != -1)
cout << "s";
if (room.east != -1)
cout << "e";
if (room.west != -1)
cout << "w";
cout << endl;
}

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
    switch (direction)
    {
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

const Room *loadRooms(const string dungeonFilename){

    ifstream fin(dungeonFilename);
    if (!fin.is_open())
    {
        cerr << "Error: failed to open file" << endl;
        exit(1);
    }

    size_t tildeCount = 0;
    string line;
    while (getline (fin, line))
    {
        if (line == "~")
        {
            tildeCount++;
        }
    }
    size_t roomCount = tildeCount / 3;
    cout << roomCount << endl;
    Room *rooms = new Room[roomCount];
    fin.clear();
    fin.seekg(0);
    string name, description, eExit;
    // Read in the rooms
    for (size_t i = 0; i < roomCount; i++) {
        getline (fin, name, '~');
        getline (fin, description, '~');

        getline (fin, eExit, '~');
        stripWhitespace(name);
        stripWhitespace(description);
        stripWhitespace(eExit);
        istringstream eStream (eExit);
        char direction;
        int roomIndex;
        while(eStream >> direction >> roomIndex) {
            setExit(rooms[i], direction, roomIndex);
        }
        setInfo(rooms[i], name, description);

    }

    dumpRooms(rooms, roomCount);
    fin.close();
    return rooms;

}

int main(int argc, char **argv) {
{
    
    if (argc != 2) {
        cerr << "Usage: ./mud filename" << endl;
        return 1;
    }

    const Room *rooms = loadRooms(argv[1]);
    if (rooms == nullptr) {
        cerr << "Error: failed to open file" << endl;
        return 1;
    }

    dumpRooms(rooms, 10);
    delete[] rooms;
    
    /* int currentRoom = 0;
    while (true) {

        char input;
        cout << "> ";
        cin >> input;

        switch (input) {

        } */
    return 0;
    }
}
