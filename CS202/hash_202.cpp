/* Program Name: Hash Tables
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description:  */
#include "hash_202.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <vector>

// Static Hash function (no longer part of the class)
static size_t Hash(const std::string &key, char Fxn, size_t table_size) {
    if (Fxn == 'L') {
        size_t result = 0;
        for (size_t i = key.size() >= 7 ? key.size() - 7 : 0; i < key.size(); ++i) {
            result = (result << 4) + (key[i] >= '0' && key[i] <= '9' ? key[i] - '0' : toupper(key[i]) - 'A' + 10);
        }
        return result % table_size;
    } else if (Fxn == 'X') {
        size_t result = 0;
        for (char c : key) {
            result ^= (c >= '0' && c <= '9' ? c - '0' : toupper(c) - 'A' + 10);
        }
        return result % table_size;
    }
    return 0;
}

// Static DoubleHash function
static size_t DoubleHash(const std::string &key, size_t table_size) {
    size_t hash_val = 0;
    for (char c : key) {
        hash_val = (hash_val << 4) + (c >= '0' && c <= '9' ? c - '0' : toupper(c) - 'A' + 10);
    }
    return (hash_val % (table_size - 1)) + 1;  // Ensure non-zero step for double hashing
}

// Method to set up the hash table with a given size, hash function, and collision strategy.
std::string Hash_202::Set_Up(size_t table_size, const std::string &fxn, const std::string &collision) {
    if (!Keys.empty()) return "Hash table already set up";
    if (table_size == 0) return "Bad table size";

    // Determine hash function
    if (fxn == "Last7") {
        Fxn = 'L';  // Last7 hash function
    } else if (fxn == "XOR") {
        Fxn = 'X';  // XOR hash function
    } else {
        return "Bad hash function";
    }

    // Determine collision strategy
    if (collision == "Linear") {
        Coll = 'L';  // Linear probing
    } else if (collision == "Double") {
        Coll = 'D';  // Double hashing
    } else {
        return "Bad collision resolution strategy";
    }

    // Initialize the hash table
    Keys.resize(table_size);
    Vals.resize(table_size);
    Nkeys = 0;

    return "";  // Successful setup
}

// Add method: inserts a key-value pair
std::string Hash_202::Add(const std::string &key, const std::string &val) {
    if (Keys.empty()) return "Hash table not set up";
    if (key.empty()) return "Empty key";
    if (val.empty()) return "Empty val";

    for (char c : key) {
        if (!isxdigit(c)) return "Bad key (not all hex digits)";
    }

    if (Nkeys == Keys.size()) return "Hash table full";

    size_t index = Hash(key, Fxn, Keys.size());
    size_t original_index = index;
    size_t double_hash = (Coll == 'D') ? DoubleHash(key, Keys.size()) : 1;

    size_t probes = 0;
    while (!Keys[index].empty() && Keys[index] != key) {
        if (++probes == Keys.size()) return "Hash table full";
        index = (original_index + probes * double_hash) % Keys.size();
    }

    Keys[index] = key;
    Vals[index] = val;
    Nkeys++;

    return "";
}

// Find method: returns the value associated with the key
std::string Hash_202::Find(const std::string &key) {
    if (Keys.empty()) return "";

    size_t index = Hash(key, Fxn, Keys.size());
    size_t original_index = index;
    size_t double_hash = (Coll == 'D') ? DoubleHash(key, Keys.size()) : 1;

    Nprobes = 0;
    while (!Keys[index].empty()) {
        Nprobes++;
        if (Keys[index] == key) {
            return Vals[index];
        }
        index = (original_index + Nprobes * double_hash) % Keys.size();
    }
    return "";  // Key not found
}

// Print method: displays the non-empty elements in the hash table
void Hash_202::Print() const {
    if (Keys.empty()) return;

    for (size_t i = 0; i < Keys.size(); ++i) {
        if (!Keys[i].empty()) {
            std::cout << std::setw(5) << i << " " << Keys[i] << " " << Vals[i] << std::endl;
        }
    }
}

// Total probes method: calculates total probes to find all keys
size_t Hash_202::Total_Probes() {
    if (Keys.empty()) return 0;

    size_t total_probes = 0;
    for (const std::string &key : Keys) {
        if (!key.empty()) {
            Find(key);  // Updates Nprobes
            total_probes += Nprobes;
        }
    }
    return total_probes;
}