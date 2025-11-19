/* Program Name: Dlist
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: Binary matrix class for creating, manipulating, and storing matrices, and includes hash-based storage */

#include "dlist.hpp"
#include <iostream>

using namespace std;

// initalize a double linked lisk wif a sentinel node
Dlist::Dlist() {
    sentinel = new Dnode;  // initalize sentinel node here
    sentinel->flink = sentinel;  // link in front points to itself
    sentinel->blink = sentinel;  // link in da back points to itself
    size = 0;  // initalize size to zero
}

// Clears the list and deallocates the sentinel node
Dlist::~Dlist() {
    Clear();  // Remove all nodes from the list
    delete sentinel;  // Delete the sentinel node
}

// Copy constructor
// Creates a new list that is a copy of an existing list
Dlist::Dlist(const Dlist &d) {
    sentinel = new Dnode;  // Create a new sentinel node
    sentinel->flink = sentinel;  // Forward link points to itself
    sentinel->blink = sentinel;  // Backward link points to itself
    size = 0;  // Initialize the size of the list to zero
    *this = d;  // Use assignment operator to copy elements from the given list
}

// Assignment operator
// Assigns the contents of one list to another
Dlist& Dlist::operator=(const Dlist &d) {
    if (this != &d) {  // Prevent self-assignment
        Clear();  // Clear the current list
        for (Dnode *current = d.Begin(); current != d.End(); current = current->Next()) {
            Push_Back(current->s);  // Add each element from the given list to the current list
        }
    }
    return *this;  // Return the current list
}

// Clear the list
// Removes all nodes from the list except the sentinel node
void Dlist::Clear() {
    Dnode *current = sentinel->flink;  // Start from the first node
    while (current != sentinel) {  // Traverse until the sentinel is reached
        Dnode *temp = current;  // Store the current node
        current = current->flink;  // Move to the next node
        delete temp;  // Delete the stored node
    }
    sentinel->flink = sentinel;  // Reset forward link to point to sentinel
    sentinel->blink = sentinel;  // Reset backward link to point to sentinel
    size = 0;  // Set size to zero
}

// Check if the list is empty
// Returns true if the list is empty, false otherwise
bool Dlist::Empty() const {
    return size == 0;  // List is empty if size is zero
}

// Return the size of the list
// Returns the number of elements in the list
size_t Dlist::Size() const {
    return size;  // Return the current size of the list
}

// Push a string to the front of the list
// Inserts a new node containing the given string at the front of the list
void Dlist::Push_Front(const string &s) {
    Insert_After(s, sentinel);  // Insert the new node after the sentinel
}

// Push a string to the back of the list
// Inserts a new node containing the given string at the back of the list
void Dlist::Push_Back(const string &s) {
    Insert_Before(s, sentinel);  // Insert the new node before the sentinel
}

// Pop a string from the front of the list
// Removes and returns the string from the front of the list
string Dlist::Pop_Front() {
    if (Empty()) {  // Check if the list is empty
        throw underflow_error("Pop_Front called on an empty list");  // Throw an error if empty
    }
    Dnode *node = sentinel->flink;  // Get the first node
    string value = node->s;  // Store the value of the first node
    Erase(node);  // Remove the first node from the list
    return value;  // Return the stored value
}

// Pop a string from the back of the list
// Removes and returns the string from the back of the list
string Dlist::Pop_Back() {
    if (Empty()) {  // Check if the list is empty
        throw underflow_error("Pop_Back called on an empty list");  // Throw an error if empty
    }
    Dnode *node = sentinel->blink;  // Get the last node
    string value = node->s;  // Store the value of the last node
    Erase(node);  // Remove the last node from the list
    return value;  // Return the stored value
}

// Return a pointer to the first node
// Returns a pointer to the first element in the list
Dnode* Dlist::Begin() const {
    return sentinel->flink;  // Return the node after the sentinel
}

// Return a pointer to one past the last node
// Returns a pointer to the sentinel node, indicating the end of the list
Dnode* Dlist::End() const {
    return sentinel;  // The sentinel represents the end of the list
}

// Return a pointer to the last node
// Returns a pointer to the last element in the list
Dnode* Dlist::Rbegin() const {
    return sentinel->blink;  // Return the node before the sentinel
}

// Return a pointer to one before the first node
// Returns a pointer to the sentinel node, indicating the reverse end of the list
Dnode* Dlist::Rend() const {
    return sentinel;  // The sentinel represents the reverse end of the list
}

// Insert a new node before a given node
// Inserts a new node containing the given string before the specified node
void Dlist::Insert_Before(const string &s, Dnode *n) {
    Dnode *newNode = new Dnode;  // Create a new node
    newNode->s = s;  // Set the value of the new node
    newNode->flink = n;  // Set the forward link of the new node to the given node
    newNode->blink = n->blink;  // Set the backward link of the new node to the previous node
    n->blink->flink = newNode;  // Update the forward link of the previous node
    n->blink = newNode;  // Update the backward link of the given node
    ++size;  // Increase the size of the list
}

// Insert a new node after a given node
// Inserts a new node containing the given string after the specified node
void Dlist::Insert_After(const string &s, Dnode *n) {
    Dnode *newNode = new Dnode;  // Create a new node
    newNode->s = s;  // Set the value of the new node
    newNode->flink = n->flink;  // Set the forward link of the new node to the next node
    newNode->blink = n;  // Set the backward link of the new node to the given node
    n->flink->blink = newNode;  // Update the backward link of the next node
    n->flink = newNode;  // Update the forward link of the given node
    ++size;  // Increase the size of the list
}

// Erase a given node from the list
// Removes the specified node from the list
void Dlist::Erase(Dnode *n) {
    if (n == sentinel) {  // Check if the node is the sentinel
        throw invalid_argument("Cannot erase sentinel node");  // Cannot erase the sentinel
    }
    n->blink->flink = n->flink;  // Update the forward link of the previous node
    n->flink->blink = n->blink;  // Update the backward link of the next node
    delete n;  // Delete the specified node
    --size;  // Decrease the size of the list
}

// Next method for Dnode
// Returns a pointer to the next node in the list
Dnode* Dnode::Next() {
    return flink;  // Return the forward link of the current node
}

// Prev method for Dnode
// Returns a pointer to the previous node in the list
Dnode* Dnode::Prev() {
    return blink;  // Return the backward link of the current node
}
