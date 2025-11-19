#include "volsort.h"

void split(Node *head, Node *&left, Node *&right) {
    Node *slow = head, *fast = head->next;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    left = head;
    right = slow->next;
    slow->next = nullptr;
}

Node *merge(Node *left, Node *right, bool numeric) {
    Node dummy;
    Node *tail = &dummy;

    while (left && right) {
        if ((numeric && left->number <= right->number) ||
            (!numeric && left->string <= right->string)) {
            tail->next = left;
            left = left->next;
        } else {
            tail->next = right;
            right = right->next;
        }
        tail = tail->next;
    }
    tail->next = left ? left : right;
    return dummy.next;
}

Node *msort(Node *head, bool numeric) {
    if (!head || !head->next) return head;
    Node *left, *right;
    split(head, left, right);
    left = msort(left, numeric);
    right = msort(right, numeric);
    return merge(left, right, numeric);
}

void merge_sort(List &l, bool numeric) {
    l.head = msort(l.head, numeric);
}