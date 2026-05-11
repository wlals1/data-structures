/*
 * avl_tree.h
 *
 * AVL Tree implementation in C.
 *
 * An AVL tree is a self-balancing binary search tree where the heights
 * of the left and right subtrees of any node differ by at most 1.
 *
 * Invariants:
 *   - BST property: left subtree keys < node key < right subtree keys.
 *   - Balance factor (left height - right height) is in {-1, 0, 1}.
 *   - Height convention: leaf height = 1, NULL height = 0.
 *
 * Time complexity: O(log n) for search, insert, and delete.
 *
 * API style: pattern A (return value updates root).
 *   Node *root = NULL;
 *   root = avl_insert(root, 42);
 *   root = avl_delete(root, 42);
 *   avl_destroy(root);
 */

#ifndef AVL_TREE_H
#define AVL_TREE_H

#include <stdbool.h>

/* === Data structure === */

typedef struct Node {
    int key;
    int height;
    struct Node *left;
    struct Node *right;
} Node;

/* === Public API === */

/**
 * Insert a key into the tree and return the (possibly new) root.
 * Duplicate keys are ignored; the existing tree is returned unchanged.
 *
 * Usage: root = avl_insert(root, key);
 */
Node *avl_insert(Node *root, int key);

/**
 * Delete a key from the tree and return the (possibly new) root.
 * If the key is not present, the tree is returned unchanged.
 *
 * Usage: root = avl_delete(root, key);
 */
Node *avl_delete(Node *root, int key);

/**
 * Return true if the key is present in the tree, false otherwise.
 */
bool avl_search(const Node *root, int key);

/**
 * Free all nodes in the tree.
 *
 * Caller should set their root pointer to NULL after calling this,
 * since this function does not modify the caller's variable.
 */
void avl_destroy(Node *root);

/**
 * Print the tree structure to stdout (for debugging).
 * Right subtree is printed first (sideways layout).
 */
void avl_print(const Node *root);

#endif /* AVL_TREE_H */
