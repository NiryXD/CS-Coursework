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

// Function to count the number of rooms in the file
int countRooms(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: failed to open file" << endl; // Print file error message
        return 0;
    }

    int tildeCount = 0;
    string line;
    while (getline(file, line)) {
        if (line == "~") {
            tildeCount++;
        }
    }

    file.close();

    // Each room has 3 tildes, so divide by 3 to get the room count
    return tildeCount / 3;
}

// Function to read rooms from the file and store them in memory
Room* loadRooms(const string& filename, int roomCount) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: failed to open file" << endl;
        return nullptr; // Return nullptr if file failed to open
    }

    Room* rooms = new Room[roomCount];
    for (int i = 0; i < roomCount; ++i) {
        getline(file, rooms[i].name);
        getline(file, rooms[i].description);

        string exitsString;
        getline(file, exitsString);

        istringstream exitsStream(exitsString);
        char direction;
        int roomIndex;
        while (exitsStream >> direction >> roomIndex) {
            switch (direction) {
                case 'n':
                    rooms[i].north = roomIndex;
                    break;
                case 's':
                    rooms[i].south = roomIndex;
                    break;
                case 'e':
                    rooms[i].east = roomIndex;
                    break;
                case 'w':
                    rooms[i].west = roomIndex;
                    break;
                default:
                    cerr << "Error: Invalid exit direction in room " << i << endl;
                    break;
            }
        }
    }

    file.close();
    return rooms;
}

// Function to deallocate memory for rooms
void deleteRooms(Room* rooms) {
    delete[] rooms;
}

void look(const Room &room) {
    cout << "Name: " << room.name << endl;
    cout << "Description: " << room.description << endl;
    cout << "Exits: ";
    if (room.north != -1) cout << "north ";
    if (room.south != -1) cout << "south ";
    if (room.east != -1) cout << "east ";
    if (room.west != -1) cout << "west ";
    cout << endl;
}

// Function to print the details of each room
void dumpRooms(const Room* rooms, int roomCount) {
    for (int i = 0; i < roomCount; ++i) {
        cout << "Room " << i << endl;
        cout << "  name: " << rooms[i].name << endl;
        cout << "  description: " << rooms[i].description << endl;
        cout << "  north: " << (rooms[i].north != -1 ? "Exists" : "None") << endl;
        cout << "  south: " << (rooms[i].south != -1 ? "Exists" : "None") << endl;
        cout << "  east:  " << (rooms[i].east != -1 ? "Exists" : "None") << endl;
        cout << "  west:  " << (rooms[i].west != -1 ? "Exists" : "None") << endl;
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        cerr << "Usage: ./mud filename" << endl; // Print usage message
        return 1;
    }

    string filename = argv[1];

    // Check if the file opens successfully
    int numRooms = countRooms(filename);
    if (numRooms == 0) {
        return 1; // Exit program if file failed to open
    }

    // Load rooms from the file into memory
    Room* rooms = loadRooms(filename, numRooms);
    if (rooms == nullptr) {
        cerr << "Error: failed to open file" << endl; // Print file error message
        return 1;
    }

    // Print room details
    dumpRooms(rooms, numRooms);

    // Cleanup: Deallocate memory
    deleteRooms(rooms);

    return 0;
}
