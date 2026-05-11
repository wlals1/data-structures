/*
 * two_three_tree.h
 *
 * 2-3 Tree implementation in C.
 *
 * A 2-3 tree is a self-balancing search tree where every internal node
 * has either 2 or 3 children, and all leaves are at the same depth.
 *
 * Invariants:
 *   - Each node holds 1 or 2 keys (2-node or 3-node).
 *   - Number of children = number of keys + 1 (for internal nodes).
 *   - All leaves have the same depth.
 *   - Keys within a node are sorted in ascending order.
 *   - For a node with keys [k0, k1] and children [c0, c1, c2]:
 *       keys in c0 < k0 < keys in c1 < k1 < keys in c2.
 *
 * Time complexity: O(log n) for search, insert, and delete.
 */

#ifndef TWO_THREE_TREE_H
#define TWO_THREE_TREE_H

#include <stdbool.h>

/* === Constants === */

#define TT_MAX_KEYS     2   /* Maximum keys per node (2 for 3-node) */
#define TT_MAX_CHILDREN 3   /* Maximum children per node            */

/* === Data structures === */

/**
 * Node of a 2-3 tree.
 *
 * A node is a 2-node when num_keys == 1 (1 key, 2 children),
 * or a 3-node when num_keys == 2 (2 keys, 3 children).
 * For leaf nodes, all children pointers are NULL.
 */
typedef struct Node {
    int keys[TT_MAX_KEYS];
    struct Node *children[TT_MAX_CHILDREN];
    int num_keys;
} Node;

/**
 * Result of a split operation during insertion.
 *
 * When a node overflows during insert, the middle key is promoted to
 * the parent and a new right-sibling node is created. If no split
 * occurs, new_right is NULL.
 */
typedef struct {
    int middle_key;
    Node *new_right;
} SplitResult;

/* === Public API === */

/**
 * Insert a key into the tree. If the tree is empty, a new root is created.
 * The root pointer may be updated if a new root is needed after split.
 * Duplicate keys are not allowed (caller's responsibility to avoid).
 */
void tree_insert(Node **root, int key);

/**
 * Delete a key from the tree. If the key is not found, the tree is
 * unchanged. The root pointer may be updated if height decreases.
 */
void tree_delete(Node **root, int key);

/**
 * Free all nodes in the tree and set *root to NULL.
 */
void tree_destroy(Node **root);

/**
 * Print the tree structure to stdout (for debugging).
 */
void tree_print(const Node *root);

#endif /* TWO_THREE_TREE_H */
