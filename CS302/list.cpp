#include "volsort.h"
#include <cstdlib>

// Constructor: Initialize the linked list
List::List() : head(nullptr), size(0) {}

// Destructor: Clean up all nodes in the list
List::~List() {
    while (head) {
        Node *temp = head;
        head = head->next;
        delete temp;
    }
}

// Push front method: Add a new node to the front of the list
void List::push_front(const std::string &s) {
    Node *new_node = new Node();
    new_node->string = s;
    try {
        new_node->number = std::stoi(s); // Convert to integer if possible
    } catch (...) {
        new_node->number = 0; // Default to 0 if conversion fails
    }
    new_node->next = head;
    head = new_node;
    size++;
}