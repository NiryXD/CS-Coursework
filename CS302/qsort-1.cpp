// qsort.cpp

#include "volsort.h"

#include <cstdlib>
#include <array>
#include <iostream>
int compare_strings(const void *a, const void *b)
{ // Snake_case instead of camelCase to match qsort_sort
    const Node *na = *(const Node **)a;
    // Assign to node pointers
    const Node *nb = *(const Node **)b;
    // Compares em
    return na->string.compare(nb->string);
}

int compare_numbers(const void *a, const void *b)
{
    const Node *na = *(const Node **)a;
    // Assign to node pointers
    const Node *nb = *(const Node **)b;
    // Determine order
    return na->number - nb->number;
}
void qsort_sort(List &l, bool numeric)
{
    if (l.size == 0)
        return;
    // Return if empty

    Node **nodes = new Node *[l.size];
    Node *current = l.head;
    // Create an array to store things temporarily
    size_t i = 0;

    // Copy linked list into the array
    while (current != nullptr)
    {
        nodes[i++] = current;
        current = current->next;
    }

    qsort(nodes, l.size, sizeof(Node *),
          numeric ? compare_numbers : compare_strings);
    // utilization of the ternary operator here
    // if it's numeric use compare number, else compare strings

    for (size_t i = 0; i < l.size - 1; i++)
    // Remake the linked list
    {
        nodes[i]->next = nodes[i + 1];
        // Links da nodes
    }
    nodes[l.size - 1]->next = nullptr;

    l.head = nodes[0];
    // Update the head ^

    delete[] nodes;
    // Helps with memory
}
