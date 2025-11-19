// Names: Tristan Lopez, Noah Cash, Jonathan Lange, Ar-Raniry Ar-Rasyid
// Overview: Merge program for sorting

#include "volsort.h"

// Function prototypes
Node *msort(Node *head, bool numeric); 
void split(Node *head, Node *&left, Node *&right); 
Node *merge(Node *left, Node *right, bool numeric); 

// start merge sort
void merge_sort(List &l, bool numeric) {
    l.head = msort(l.head, numeric);
}

// sorts linked list
Node *msort(Node *head, bool numeric) {
    if (!head || !head->next) {
        return head; // if list is empty / has one node
    }

    Node *left, *right;
    split(head, left, right); // split list into two

    // sort both halves
    left = msort(left, numeric);
    right = msort(right, numeric);

    // merge halves
    return merge(left, right, numeric);
}

// splits the list into two
void split(Node *head, Node *&left, Node *&right) {
    // fast and slow pointer for efficiency
    Node *slow = head;
    Node *fast = head->next;

    // move fast pointer twice as fast as slow pointer
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }

    // left is from head to middle, right is from middle to end
    left = head;
    right = slow->next;
    slow->next = nullptr;
}

// merges two halves into one
Node *merge(Node *left, Node *right, bool numeric) {
    Node temp; // temporary head
    Node *tail = &temp; // points to last node

    while (left && right) {
        // left
        if ((numeric && left->number <= right->number) || (!numeric && left->string <= right->string)) {
            tail->next = left; // add smaller node to merged list
            left = left->next; // move to next node in left
        } 
        // right
        else {
            tail->next = right; // add smaller node to merged list
            right = right->next; // move to next node in right
        }
        tail = tail->next; 
    }

    // add leftover nodes
    if (left) {
        tail->next = left;
    } 
    else {
        tail->next = right;
    }    
    
    Node *merged = temp.next;
    return merged;
}
