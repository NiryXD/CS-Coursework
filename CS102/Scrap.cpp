    /* Program Name: Multi-User Dungeons
    Student Name: Ar-Raniry Ar-Rasyid
    NED ID: jzr266
    Student ID: 000-663-921
    Program Description: An interactable text adventure functioning through the use of pounters*/

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
            return -1;
        }
    }

    // I utilize a swith statement to take in user input to return the corresponding direction.

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
            cerr << "Error" << endl;
            break;
        }

    // Applying the same line of reasoing from before, just for exits this time around
    // There's also a error check at the end

    }

    // Take in a name and description and assign them to the room.
    void setInfo(Room &room, const string &name, const string &description) {
        room.name = name;
        room.description = description;
    }

    // Taking in known varibales and assigning them to the setInfo object.

    void look(const Room &room) {
    cout << room.name << endl;
    cout << room.description << endl;
    cout << endl;
    cout << "Exits:";
    if (room.north != -1)
    cout << " n";
    if (room.south != -1)
    cout << " s";
    if (room.east != -1)
    cout << " e";
    if (room.west != -1)
    cout << " w";
    cout << endl;

    }

    // The look object

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

    // dumpRooms and stripWhitespace are given

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

    const Room *loadRooms(const string dungeonFilename) {
        ifstream fin(dungeonFilename);
        if (!fin.is_open())
        {
            cerr << "Error: failed to open file" << endl;
            exit (1);
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
        Room *rooms = new Room[roomCount];
        fin.clear();
        fin.seekg(0);
        string name, description, exit;

        for (size_t i = 0; i < roomCount; i++) {
            getline (fin, name, '~');
            getline (fin, description, '~');

            getline (fin, exit, '~');
            stripWhitespace(name);
            stripWhitespace(description);
            stripWhitespace(exit);
            istringstream stream (exit);
            char direction;
            int index;
            while (stream >> direction >> index) {
                setExit(rooms[i], direction, index);
            }
            setInfo(rooms[i], name, description);

        }

        dumpRooms(rooms, roomCount);
        fin.close();
        return rooms;

    }

    int main(int argc, char **argv) {
    if (argc != 2) {
        cerr << "Usage: ./mud filename" << endl;
        return 1;
    }

    const Room *rooms = loadRooms(argv[1]);
    if (rooms == nullptr) {
        cerr << "Error: failed to open file" << endl;
        return 1;
    }

    int currentRoom = 0;

    while (true) {

        char input;
        cout << "> ";
        cin >> input;

        switch (input) {
            case 'q':
                // Quit the game
                delete[] rooms; // Free memory
                return 0; // Return 0 to indicate success and exit the program
            case 'l':
                // Look command
                look(rooms[currentRoom]);
                break;
            case 'n':
            case 's':
            case 'e':
            case 'w':
                // Navigate command
                int nextRoomIndex = getExit(rooms[currentRoom], input);
                if (nextRoomIndex != -1) {
                    currentRoom = nextRoomIndex;
                    cout << "You moved " << getDirectionName(input) << "." << endl;
                } else {
                    cout << "You can't go " << getDirectionName(input) << "!" << endl;
                }
                break;
        }
    }
    }
    