/*
 * rb_tree.h
 *
 * Red-Black Tree implementation in C.
 *
 * A red-black tree is a self-balancing binary search tree. Each node is
 * colored red or black, and a set of color invariants keeps the longest
 * root-to-leaf path at most twice the shortest, guaranteeing O(log n)
 * height. Compared to an AVL tree it balances more loosely, which makes
 * its rebalancing cheaper: an insert needs at most 2 rotations and a
 * delete at most 3, whereas an AVL delete may rotate O(log n) times.
 *
 * Invariants:
 *   1. Every node is either red or black.
 *   2. The root is black.
 *   3. Every NIL (leaf sentinel) is black.
 *   4. A red node has no red child (no two reds in a row).
 *   5. Every root-to-NIL path through a given node has the same number
 *      of black nodes (the node's "black-height").
 *
 * Time complexity: O(log n) for search, insert, and delete.
 *
 * Empty tree / sentinel:
 *   This implementation uses a single shared sentinel, NIL, to represent
 *   every empty position (including an empty tree). NIL is always black.
 *   Callers must initialize an empty tree to NIL, not NULL:
 *
 *       Node *root = NIL;
 *
 * API style: pattern B (the root is passed by address and updated in
 *   place). This differs from the AVL tree in this repository, which
 *   uses pattern A (return value). Pattern B is the natural choice here
 *   because nodes carry parent pointers, so the functions can relink the
 *   whole tree themselves and the caller never has to reassign the root.
 *
 *       Node *root = NIL;
 *       rb_insert(&root, 42);
 *       rb_delete(&root, 42);
 *       rb_destroy(&root);
 */

#ifndef RB_TREE_H
#define RB_TREE_H

#include <stdbool.h>

/* === Data structure === */

typedef enum {
    RED,
    BLACK
} Color;

typedef struct Node {
    int           key;
    Color         color;
    struct Node  *left;
    struct Node  *right;
    struct Node  *parent;
} Node;

/*
 * The shared leaf sentinel. Every empty child position and the empty
 * tree itself are represented by NIL. It is always black and must never
 * be freed or recolored. Defined in rb_tree.c.
 */
extern Node *NIL;

/* === Public API === */

/**
 * Insert a key into the tree.
 * Duplicate keys are ignored; the tree is left unchanged.
 *
 * Usage: rb_insert(&root, key);
 */
void rb_insert(Node **root, int key);

/**
 * Delete a key from the tree.
 * If the key is not present, the tree is left unchanged.
 *
 * Usage: rb_delete(&root, key);
 */
void rb_delete(Node **root, int key);

/**
 * Return true if the key is present in the tree, false otherwise.
 */
bool rb_search(const Node *root, int key);

/**
 * Free every node in the tree and reset *root to NIL.
 * The NIL sentinel itself is not freed.
 *
 * Usage: rb_destroy(&root);
 */
void rb_destroy(Node **root);

/**
 * Verify that the tree satisfies every red-black and BST invariant.
 * Returns true if valid. Intended for testing and debugging.
 */
bool rb_is_valid(const Node *root);

/**
 * Print the tree structure to stdout (for debugging).
 * The right subtree is printed first (sideways layout); each node shows
 * its key and color, e.g. "26(R)".
 */
void rb_print(const Node *root);

#endif /* RB_TREE_H */
