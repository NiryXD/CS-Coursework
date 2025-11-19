/* Program Name: Hash Tables
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description:  Creating a Hash table to store and retrieve data*/
#include "hash_202.hpp"
#include <sstream>
#include <iostream>
#include <iomanip>

using namespace std;

// Initalizing static functions
static size_t Hash_Function_Last7 (const string &key);
// Above is a function to hash the last 7 characters
static size_t Hash_Function_XOR (const string &key);
// Above is function to hash using XOR

string Hash_202::Set_Up(size_t table_size, const string &fxn, const string &collision) {
    // Checks to see if there's an already existing hash table
    if (!Keys.empty()) return "Hash table already set up";
    // Check if the size is allowed
    if (table_size == 0) return "Bad table size";

    // Resize the vectors to the table size given and then initialize them
    Keys.resize(table_size, "");
    Vals.resize(table_size, "");

    // Choosing which function to use based on the given string
    if (fxn == "Last7") {
        Fxn = 'L';
    } else if (fxn == "XOR") {
        Fxn = 'X';
    } else {
        return "Bad hash function";
    }
    
    // Setting up collision resolution based on the given string
    if (collision == "Linear") {
        Coll = 'L';
    } else if (collision == "Double") {
        Coll = 'D';
    } else {
        return "Bad collision resolution strategy";
    }
    
    return "";
    }

    static size_t Hash_Function_Last7 (const string &key) {
        // Get the last 7 chars of the key
        size_t len = key.length();
        // Right here determines the length of the key and stores it into len
        size_t start = len >= 7 ? len - 7 : 0;
        // Utilizing a ternary operator we can take the last 7 chars, but if 
        // the key is shorter than 7 to begin with we'll just take the whole thing
        istringstream ss(key.substr(start));
        // utilizing stringstream from <sstream> we can pull the last 7 chars.
        size_t hash_value = 0;
        ss >> hex >> hash_value;
        // here we get 'ss' into out initalized variable, but before that I used
        // hex to convert the substring to a decimal equivalent
        return hash_value;
    }

    static size_t Hash_Function_XOR (const string &key) {
        // XOR the hash values of the last 7 characters
        size_t hash_value = 0;
        for (size_t i = 0; i < key.length(); i = i + 7) {
        // I utilize a loop here that itterates over the 7 chars
            istringstream ss(key.substr(i, 7));
        // Doing the same thing from the Last7 function but for chars i to 7
            size_t part;
            ss >> hex >> part;
            hash_value = hash_value ^ part;
        // Right here is the XOR function
        }
        return hash_value;
    }

    string Hash_202 :: Add (const string &key, const string &val) {
        // Checks to see if there's an already existing hash table
        if (Keys.empty()) return "Hash table not set up";
        // Checks if key is empty
        if (key.empty()) return "Empty key";
        // Checks if value is empty
        if (val.empty()) return "Empty val";

        // Checks if chars are hexadecimal
        for (char c : key) {
            if (!isxdigit(c)) return "Bad key (not all hex digits)";
            // Right here is an important distinction, I'm using 'isxdigit' which recognizes not only numberic digits
            // But also hecadecimal characters like a-f and A-F
        }

        size_t table_size = Keys.size();
        // Find index using hash function
        size_t index = (Fxn == 'L') ? Hash_Function_Last7(key) % table_size : Hash_Function_XOR(key) % table_size;
        // Using ternary operator to complete this, I think it's faster to write than an if else statement. 
        size_t step = (Coll == 'D') ? ((Fxn == 'L') ?  Hash_Function_XOR(key) % table_size : Hash_Function_Last7(key) % table_size) : 1;
        // Above calculates the step size for double hashing, else set it to 1 for linear probing
        if (step == 0) 
        step = 1;

        size_t probes = 0;
        while (probes < table_size) {
            // Above uses a loop to find and empty stop in the table, Below recognizes if it is
            if (Keys[index].empty() || Keys[index] == key) {
                if (Keys[index] == key) return "Key already in the table";
                // else do below:
                Keys[index] = key;
                Vals[index] = val;
                Nkeys++;
                return "";
            }
            // Update the new index
            index = (index + step) % table_size;
            probes++;
        }

        return "Hash table full";
    }

    string Hash_202 :: Find (const string &key) {
        // Checks to see if there's an  existing hash table
        if (Keys.empty()) return "";
        // Check to see if key is empty
        if (key.empty()) return "";

        // Check if the chars are hexadecimal
        for (char c : key) {
            if (!isxdigit(c)) return "";
        }

        size_t table_size = Keys.size();
        // Find index using hash function
        size_t index = (Fxn == 'L') ? Hash_Function_Last7(key) % table_size : Hash_Function_XOR(key) % table_size;
        // Using ternary operator to complete this, I think it's faster to write than an if else statement. 
        size_t step = (Coll == 'D') ? ((Fxn == 'L') ?  Hash_Function_XOR(key) % table_size : Hash_Function_Last7(key) % table_size) : 1;
        // Above calculates the step size for double hashing, else set it to 1 for linear probing
        if (step == 0) 
        step = 1;

        Nprobes = 0;
        while (Nprobes < table_size) {
            // this is kinda the Add function
            if (Keys[index].empty()) return "";
            if (Keys[index] == key) return Vals[index];
            index = (index + step) % table_size;
            Nprobes++;
        } 
        // This searches for a key in the hash table. Starts with Nprobes and loops throught the table.

        return "";

    }

    void Hash_202 :: Print () const {
        // Check if there's a hash table
        if (Keys.empty()) return ;
        // Loop through the hash table and print the slots with data in em
        for (size_t i = 0; i < Keys.size(); i++) {
            if (!Keys[i].empty()) {
                cout << right << setw(5) << i << " " << Keys[i] << " " << Vals[i] << endl;
            }
        }
    }

    size_t Hash_202 :: Total_Probes () {
        // Check if there's a hash table
        if (Keys.empty()) return 0;
        size_t all_probes = 0;
        // Loop through the hash table and count the probes
        for (const string &key : Keys) {
            if (!key.empty()) {
                Find(key);
                all_probes = all_probes + Nprobes;
            }
        }
        return all_probes;
    }
