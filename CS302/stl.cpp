#include "volsort.h"
#include <vector>
#include <algorithm>

bool node_string_compare(const Node *a, const Node *b) {
    return a->string < b->string;
}

bool node_number_compare(const Node *a, const Node *b) {
    return a->number < b->number;
}

void stl_sort(List &l, bool numeric) {
    std::vector<Node *> nodes;
    for (Node *curr = l.head; curr; curr = curr->next) {
        nodes.push_back(curr);
    }

    if (numeric) {
        std::sort(nodes.begin(), nodes.end(), node_number_compare);
    } else {
        std::sort(nodes.begin(), nodes.end(), node_string_compare);
    }

    l.head = nodes[0];
    for (size_t i = 0; i < nodes.size() - 1; ++i) {
        nodes[i]->next = nodes[i + 1];
    }
    nodes.back()->next = nullptr;
}