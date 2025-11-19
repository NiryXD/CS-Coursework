// Names: Tristan Lopez, Noah Cash, Jonathan Lange, Ar-Raniry Ar-Rasyid
// Overview: STL program for sorting
#include "volsort.h"
#include <algorithm>
#include <vector>
#include <iostream>
using namespace std;

// C++ style comparison functions
bool node_string_compare(const Node *a, const Node *b) {
    return a->string < b->string; // true if string is in correct order (ascending)
}
// C++ style comparisons functions
bool node_number_compare(const Node *a, const Node *b) {
    return a->number < b->number; // true if number is in correct order (ascending)
}
// void stl function
void stl_sort(List &l, bool numeric) {
   // the nodes are stored into a pointer
    vector<Node*> nodes;
    Node *index = l.head;
   // the pointer is filled in with nodes from list
    while (index != nullptr) {
        nodes.push_back(index); // nodes are pushed back
        index = index->next;
    }
    // vector sorting methods
    if (numeric) {
        // sort the values if they are in the correct format
        sort(nodes.begin(), nodes.end(), node_number_compare);
    } else {
        // sort the values if they are in the wrong format
        sort(nodes.begin(), nodes.end(), node_string_compare);
    }
   // the nodes are put back in order
    for (size_t i = 0; i < nodes.size() - 1; i++) {
        nodes[i]->next = nodes[i + 1];
    }
    nodes.back()->next = nullptr; // the last node of the list is nulled
    // the list is updated back into sorted order
    l.head = nodes[0];
}
