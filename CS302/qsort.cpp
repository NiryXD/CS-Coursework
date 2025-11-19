#include "volsort.h"
#include <cstdlib>
#include <vector>

int qComparisonNumber(const void *a, const void *b) {
    const Node *ia = *(const Node **)a;
    const Node *ib = *(const Node **)b;
    return ia->number - ib->number;
}

int qComparisonString(const void *a, const void *b) {
    const Node *ia = *(const Node **)a;
    const Node *ib = *(const Node **)b;
    return ia->string.compare(ib->string);
}

void qsort_sort(List &l, bool numeric) {
    std::vector<Node *> nodes;
    for (Node *curr = l.head; curr; curr = curr->next) {
        nodes.push_back(curr);
    }

    if (numeric) {
        qsort(nodes.data(), nodes.size(), sizeof(Node *), qComparisonNumber);
    } else {
        qsort(nodes.data(), nodes.size(), sizeof(Node *), qComparisonString);
    }

    l.head = nodes[0];
    for (size_t i = 0; i < nodes.size() - 1; ++i) {
        nodes[i]->next = nodes[i + 1];
    }
    nodes.back()->next = nullptr;
}