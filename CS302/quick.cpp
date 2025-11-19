// quick.cpp

#include "volsort.h"

// Function Prototypes
Node *qsort(Node *head, bool numeric);
void partition(Node *head, Node *pivot, Node *&left, Node *&right, bool numeric);
Node *concatenate(Node *left, Node *pivot, Node *right);

// Quick Sort Implementation
void quick_sort(List &l, bool numeric) {
    l.head = qsort(l.head, numeric); // Perform the quick sort
}

// Recursive Quick Sort Function
Node *qsort(Node *head, bool numeric) {
    if (!head || !head->next) {
        return head; // Base case: empty or single-node list
    }

    Node *pivot = head; // Choose the first node as the pivot
    Node *left = nullptr, *right = nullptr;

    partition(head->next, pivot, left, right, numeric); // Partition the list
    left = qsort(left, numeric);  // Recursively sort left sublist
    right = qsort(right, numeric); // Recursively sort right sublist

    return concatenate(left, pivot, right); // Combine sorted lists
}

// Partition the list into left and right sublists
void partition(Node *head, Node *pivot, Node *&left, Node *&right, bool numeric) {
    left = nullptr;
    right = nullptr;

    while (head) {
        Node *next = head->next; // Save the next node
        if ((numeric && head->number < pivot->number) ||
            (!numeric && head->string < pivot->string)) {
            head->next = left;
            left = head;
        } else {
            head->next = right;
            right = head;
        }
        head = next; // Move to the next node
    }
}

// Concatenate the left list, pivot, and right list
Node *concatenate(Node *left, Node *pivot, Node *right) {
    pivot->next = right; // Attach pivot to the right list

    if (!left) { // If left list is empty, return pivot + right
        return pivot;
    }

    Node *tail = left;
    while (tail->next) {
        tail = tail->next; // Traverse to the end of the left list
    }
    tail->next = pivot; // Attach pivot to the end of the left list
    return left;
}