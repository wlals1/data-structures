#ifndef B_TREE_H
#define B_TREE_H

#include <stdbool.h>

/*
 * B-tree (CLRS-style, parameterized by minimum degree T).
 *
 * The minimum degree T (>= 2) is the single tuning knob. Setting T fixes
 * every other bound, and keeping T as the parameter (rather than the order
 * M = 2T) guarantees an even maximum number of children, so node splits are
 * always symmetric (T-1 keys on each side) and never violate the occupancy
 * lower bound.
 *
 * Per-node bounds:
 *   - keys:     MIN_KEYS .. MAX_KEYS   (T-1 .. 2T-1)
 *   - children: T .. 2T                (one more than the number of keys)
 *   The root is exempt from the MIN_KEYS lower bound; it may hold as few
 *   as 1 key (or 0 keys only transiently, when the tree becomes empty).
 *
 * Invariants (maintained by every operation):
 *   1. Keys within a node are sorted ascending.
 *   2. For a node with n keys, children[i] holds all keys strictly between
 *      keys[i-1] and keys[i] (with the obvious open ends), so the search-tree
 *      ordering holds across the whole tree.
 *   3. A non-root node has at least MIN_KEYS keys.
 *   4. All leaves are at the same depth (perfect height balance).
 *
 * Both insert and delete run top-down in a single downward pass:
 *   - Insert splits any full child *before* descending, so a split never
 *     propagates back up.
 *   - Delete refills any minimum-size child (via borrow or merge) *before*
 *     descending, so an underflow never propagates back up.
 */

#define T 3                  /* minimum degree, T >= 2                   */
#define MAX_KEYS (2 * T - 1) /* a full node holds this many keys         */
#define MIN_KEYS (T - 1)     /* a non-root node holds at least this many */
#define MAX_CHILDREN (2 * T) /* = M, the order (max children per node)   */

typedef struct BTreeNode {
  int keys[MAX_KEYS];
  struct BTreeNode *children[MAX_CHILDREN];
  int num_keys;
} Node;

/* Insert a key. Set semantics are NOT enforced: inserting a key that is
 * already present stores a duplicate, so callers that need set behavior
 * should guard with tree_search first. */
void tree_insert(Node **root, int key);

/* Delete a key. Silently does nothing if the key is absent. */
void tree_delete(Node **root, int key);

/* Return true iff key is present. */
bool tree_search(const Node *root, int key);

/* Print the tree sideways (rotated 90 degrees, largest key at the top). */
void tree_print(const Node *root);

/* Free every node and set *root to NULL. */
void tree_destroy(Node **root);

#endif /* B_TREE_H */
