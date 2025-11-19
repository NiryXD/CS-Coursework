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

// Static helper functions declaration
static size_t Hash_Function_Last7(const string &key); // Function to hash the last 7 characters of a key
static size_t Hash_Function_XOR(const string &key); // Function to hash a key using XOR operation

string Hash_202::Set_Up(size_t table_size, const string &fxn, const string &collision) {
    // Check if the hash table is already set up
    if (!Keys.empty()) return "Hash table already set up";
    // Check for a valid table size
    if (table_size == 0) return "Bad table size";

    // Resize the Keys and Vals vectors to the given table size and initialize them with empty strings
    Keys.resize(table_size, "");
    Vals.resize(table_size, "");
    
    // Set the hash function to use based on the provided string
    if (fxn == "Last7") {
        Fxn = 'L';
    } else if (fxn == "XOR") {
        Fxn = 'X';
    } else {
        return "Bad hash function";
    }

    // Set the collision resolution strategy based on the provided string
    if (collision == "Linear") {
        Coll = 'L';
    } else if (collision == "Double") {
        Coll = 'D';
    } else {
        return "Bad collision resolution strategy";
    }

    return "";
}

static size_t Hash_Function_Last7(const string &key) {
    // Get the last 7 characters of the key, or fewer if the key is shorter
    size_t len = key.length();
    size_t start = len >= 7 ? len - 7 : 0;
    istringstream ss(key.substr(start));
    size_t hash_value;
    ss >> hex >> hash_value; // Convert the substring to a hash value in hexadecimal
    return hash_value;
}

static size_t Hash_Function_XOR(const string &key) {
    // XOR the hash values of 7-character segments of the key
    size_t hash_value = 0;
    for (size_t i = 0; i < key.length(); i += 7) {
        istringstream ss(key.substr(i, 7));
        size_t part;
        ss >> hex >> part; // Convert the substring to a hash value in hexadecimal
        hash_value ^= part; // XOR the current hash value with the new part
    }
    return hash_value;
}

string Hash_202::Add(const string &key, const string &val) {
    // Check if the hash table is set up
    if (Keys.empty()) return "Hash table not set up";
    // Check if the key is empty
    if (key.empty()) return "Empty key";
    // Check if the value is empty
    if (val.empty()) return "Empty val";

    // Check if all characters in the key are hexadecimal digits
    for (char c : key) {
        if (!isxdigit(c)) return "Bad key (not all hex digits)";
    }

    size_t table_size = Keys.size();
    // Compute the initial index using the selected hash function
    size_t index = (Fxn == 'L') ? Hash_Function_Last7(key) % table_size : Hash_Function_XOR(key) % table_size;
    // Compute the step size for double hashing or set it to 1 for linear probing
    size_t step = (Coll == 'D') ? ((Fxn == 'L') ? Hash_Function_XOR(key) % table_size : Hash_Function_Last7(key) % table_size) : 1;
    if (step == 0) step = 1; // Ensure step size is not zero

    size_t probes = 0;
    // Loop to find an empty slot or the key in the table
    while (probes < table_size) {
        if (Keys[index].empty() || Keys[index] == key) {
            // If the key already exists, return an error message
            if (Keys[index] == key) return "Key already in the table";
            // Otherwise, insert the key and value into the table
            Keys[index] = key;
            Vals[index] = val;
            Nkeys++;
            return "";
        }
        // Update the index for the next probe
        index = (index + step) % table_size;
        probes++;
    }

    return "Hash table full";
}

string Hash_202::Find(const string &key) {
    // Check if the hash table is set up
    if (Keys.empty()) return "";
    // Check if the key is empty
    if (key.empty()) return "";

    // Check if all characters in the key are hexadecimal digits
    for (char c : key) {
        if (!isxdigit(c)) return "";
    }

    size_t table_size = Keys.size();
    // Compute the initial index using the selected hash function
    size_t index = (Fxn == 'L') ? Hash_Function_Last7(key) % table_size : Hash_Function_XOR(key) % table_size;
    // Compute the step size for double hashing or set it to 1 for linear probing
    size_t step = (Coll == 'D') ? ((Fxn == 'L') ? Hash_Function_XOR(key) % table_size : Hash_Function_Last7(key) % table_size) : 1;
    if (step == 0) step = 1; // Ensure step size is not zero

    Nprobes = 0;
    // Loop to find the key in the table
    while (Nprobes < table_size) {
        // If an empty slot is found, the key is not in the table
        if (Keys[index].empty()) return "";
        // If the key is found, return the corresponding value
        if (Keys[index] == key) return Vals[index];
        // Update the index for the next probe
        index = (index + step) % table_size;
        Nprobes++;
    }

    return "";
}

void Hash_202::Print() const {
    // Check if the hash table is set up
    if (Keys.empty()) return;
    // Loop through the hash table and print non-empty slots
    for (size_t i = 0; i < Keys.size(); ++i) {
        if (!Keys[i].empty()) {
            cout << right << setw(5) << i << " " << Keys[i] << " " << Vals[i] << endl;
        }
    }
}

size_t Hash_202::Total_Probes() {
    // Check if the hash table is set up
    if (Keys.empty()) return 0;
    size_t total_probes = 0;
    // Loop through the hash table and count the probes for each key
    for (const auto &key : Keys) {
        if (!key.empty()) {
            Find(key); // Find the key to count the number of probes
            total_probes += Nprobes;
        }
    }
    return total_probes;
}