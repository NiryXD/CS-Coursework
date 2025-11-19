#include <vector>
#include <string>
#include <iostream>
#include <cstdio>
#include "bstree.hpp"
using namespace std;
using CS202::BSTree;
using CS202::BSTNode;

int BSTree::Depth(const string &key) const {
  BSTNode *n = sentinel->right;
  int depth = 0;
  
  while (n != sentinel) {
    if (key == n->key) return depth;
    n = (key < n->key) ? n->left : n->right;
    depth++;
  }
  return -1;
}

int BSTree::recursive_find_height(const BSTNode *n) const {
  if (n == sentinel) return -1;
  int left_height = recursive_find_height(n->left);
  int right_height = recursive_find_height(n->right);
  return 1 + ((left_height > right_height) ? left_height : right_height);
}

int BSTree::Height() const {
  return recursive_find_height(sentinel->right) + 1;
}

void BSTree::make_key_vector(const BSTNode *n, vector<string> &v) const {
  if (n == sentinel) return;
  make_key_vector(n->left, v);
  v.push_back(n->key);
  make_key_vector(n->right, v);
}

vector<string> BSTree::Ordered_Keys() const {
  vector<string> keys;
  make_key_vector(sentinel->right, keys);
  return keys;
}

BSTNode *BSTree::make_balanced_tree(const vector<string> &sorted_keys,
                                  const vector<void *> &vals, 
                                  size_t first_index,
                                  size_t num_indices) const {
  if (num_indices == 0) return sentinel;
  
  size_t middle = first_index + (num_indices)/2;
  
  BSTNode *n = new BSTNode;
  n->key = sorted_keys[middle];
  n->val = vals[middle];
  n->parent = NULL;
  
  size_t left_size = middle - first_index;
  size_t right_size = num_indices - left_size - 1;
  
  n->left = make_balanced_tree(sorted_keys, vals, first_index, left_size);
  n->right = make_balanced_tree(sorted_keys, vals, middle + 1, right_size);
  
  if (n->left != sentinel) n->left->parent = n;
  if (n->right != sentinel) n->right->parent = n;
  
  return n;
}

BSTree::BSTree(const BSTree &t) {
  sentinel = new BSTNode;
  sentinel->parent = NULL;
  sentinel->left = NULL;
  sentinel->right = sentinel;
  sentinel->key = "---SENTINEL---";
  sentinel->val = NULL;
  size = 0;
  *this = t;
}

BSTree& BSTree::operator=(const BSTree &t) {
  if (this != &t) {
    Clear();
    if (!t.Empty()) {
      vector<string> keys = t.Ordered_Keys();
      vector<void *> vals = t.Ordered_Vals();
      
      BSTNode *root = make_balanced_tree(keys, vals, 0, keys.size());
      root->parent = sentinel;
      sentinel->right = root;
      size = t.size;
    }
  }
  return *this;
}