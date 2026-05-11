/*
 * two_three_tree.c
 *
 * 2-3 Tree implementation in C.
 * See two_three_tree.h for usage and invariants.
 */

#include "two_three_tree.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/* === Internal helpers (forward declarations) === */

static Node *create_node(int key);
static bool is_leaf(const Node *n);
static int find_key_in_node(const Node *node, int key);
static int find_child_index(const Node *node, int key);
static Node *find_min_leaf(Node *node);
static void node_insert_key(Node *node, int key);

static SplitResult split_leaf(Node *leaf, int new_key);
static SplitResult split_internal(Node *node, int new_key, Node *new_child,
                                  int idx);
static SplitResult insert_recursive(Node *node, int key);

static bool delete_recursive(Node *node, int key);
static bool resolve_underflow(Node *parent, int empty_idx);
static void redistribute(Node *parent, int empty_idx, int sibling_idx);
static void merge(Node *parent, int empty_idx, int sibling_idx);

static void print_recursive(const Node *node, int depth);

/* === Node creation === */

/**
 * Allocate a new leaf node holding a single key.
 * Aborts on allocation failure.
 */
static Node *create_node(int key) {
  Node *new_node = malloc(sizeof(*new_node));
  assert(new_node != NULL);

  new_node->keys[0] = key;
  new_node->num_keys = 1;
  for (int i = 0; i < TT_MAX_CHILDREN; i++) {
    new_node->children[i] = NULL;
  }
  return new_node;
}

/* === Node inspection === */

/**
 * Return true if the node is a leaf (no children).
 */
static bool is_leaf(const Node *n) {
  if (n == NULL)
    return false;
  return n->children[0] == NULL;
}

/**
 * Find the index of a key within a node. Returns -1 if not found.
 */
static int find_key_in_node(const Node *node, int key) {
  assert(node != NULL);
  for (int i = 0; i < node->num_keys; i++) {
    if (node->keys[i] == key)
      return i;
  }
  return -1;
}

/**
 * Determine which child to descend into for a key not present in this node.
 * Returns 0, 1, or 2 (the index into the children array).
 * Precondition: node is internal and key is not in node.
 */
static int find_child_index(const Node *node, int key) {
  assert(node != NULL);
  assert(!is_leaf(node));
  assert(find_key_in_node(node, key) == -1);

  for (int i = 0; i < node->num_keys; i++) {
    if (key < node->keys[i])
      return i;
  }
  return node->num_keys;
}

/**
 * Return the leftmost leaf in the subtree rooted at node.
 * Used to find the inorder successor for internal-key deletion.
 */
static Node *find_min_leaf(Node *node) {
  assert(node != NULL);
  while (!is_leaf(node)) {
    node = node->children[0];
  }
  return node;
}

/* === Insert helpers === */

/**
 * Insert a key into a node that has space (num_keys < 2).
 * Maintains sorted order of keys.
 */
static void node_insert_key(Node *node, int key) {
  assert(node != NULL);
  assert(node->num_keys < TT_MAX_KEYS);

  if (node->num_keys == 1 && key < node->keys[0]) {
    node->keys[1] = node->keys[0];
    node->keys[0] = key;
  } else {
    node->keys[node->num_keys] = key;
  }
  node->num_keys++;
}

/**
 * Split a full leaf node when inserting a new key.
 * The middle of the three keys is promoted; a new right sibling holds
 * the largest key.
 */
static SplitResult split_leaf(Node *leaf, int new_key) {
  assert(leaf != NULL);
  assert(is_leaf(leaf));
  assert(leaf->num_keys == TT_MAX_KEYS);
  assert(leaf->keys[0] != new_key && leaf->keys[1] != new_key);

  /* Sort the three keys */
  int temp[3];
  if (leaf->keys[1] < new_key) {
    temp[0] = leaf->keys[0];
    temp[1] = leaf->keys[1];
    temp[2] = new_key;
  } else if (leaf->keys[0] > new_key) {
    temp[0] = new_key;
    temp[1] = leaf->keys[0];
    temp[2] = leaf->keys[1];
  } else {
    temp[0] = leaf->keys[0];
    temp[1] = new_key;
    temp[2] = leaf->keys[1];
  }

  /* Original leaf keeps the smallest key */
  leaf->keys[0] = temp[0];
  leaf->num_keys = 1;

  /* New sibling holds the largest key */
  Node *new_right = create_node(temp[2]);
  return (SplitResult){.middle_key = temp[1], .new_right = new_right};
}

/**
 * Split a full internal node when a child split promotes a new key.
 * The middle key is promoted to the caller, and a new right sibling
 * takes the right portion of keys and children.
 *
 * idx is the position where the new key would be inserted (0, 1, or 2).
 */
static SplitResult split_internal(Node *node, int new_key, Node *new_child,
                                  int idx) {
  assert(node != NULL);
  assert(!is_leaf(node));
  assert(node->num_keys == TT_MAX_KEYS);
  assert(new_child != NULL);
  assert(idx >= 0 && idx <= TT_MAX_KEYS);

  /* Build sorted arrays of 3 keys and 4 children */
  int temp_keys[3];
  Node *temp_children[4];

  if (idx == 0) {
    temp_keys[0] = new_key;
    temp_keys[1] = node->keys[0];
    temp_keys[2] = node->keys[1];
    temp_children[0] = node->children[0];
    temp_children[1] = new_child;
    temp_children[2] = node->children[1];
    temp_children[3] = node->children[2];
  } else if (idx == 1) {
    temp_keys[0] = node->keys[0];
    temp_keys[1] = new_key;
    temp_keys[2] = node->keys[1];
    temp_children[0] = node->children[0];
    temp_children[1] = node->children[1];
    temp_children[2] = new_child;
    temp_children[3] = node->children[2];
  } else { /* idx == 2 */
    temp_keys[0] = node->keys[0];
    temp_keys[1] = node->keys[1];
    temp_keys[2] = new_key;
    temp_children[0] = node->children[0];
    temp_children[1] = node->children[1];
    temp_children[2] = node->children[2];
    temp_children[3] = new_child;
  }

  /* Reuse original node as the left half */
  node->keys[0] = temp_keys[0];
  node->num_keys = 1;
  node->children[0] = temp_children[0];
  node->children[1] = temp_children[1];
  node->children[2] = NULL;

  /* Allocate a new node for the right half */
  Node *new_right = malloc(sizeof(*new_right));
  assert(new_right != NULL);
  new_right->keys[0] = temp_keys[2];
  new_right->num_keys = 1;
  new_right->children[0] = temp_children[2];
  new_right->children[1] = temp_children[3];
  new_right->children[2] = NULL;

  return (SplitResult){.middle_key = temp_keys[1], .new_right = new_right};
}

/**
 * Recursive insert. Returns a SplitResult; new_right is non-NULL only
 * if a split propagated up to this node.
 */
static SplitResult insert_recursive(Node *node, int key) {
  assert(node != NULL);

  /* Leaf: insert directly, splitting if full */
  if (is_leaf(node)) {
    if (node->num_keys < TT_MAX_KEYS) {
      node_insert_key(node, key);
      return (SplitResult){0, NULL};
    }
    return split_leaf(node, key);
  }

  /* Internal: descend into the appropriate child */
  int idx;
  if (key < node->keys[0]) {
    idx = 0;
  } else if (node->num_keys == 1 || key < node->keys[1]) {
    idx = 1;
  } else {
    idx = 2;
  }

  SplitResult child_result = insert_recursive(node->children[idx], key);

  /* No split below; nothing to do */
  if (child_result.new_right == NULL) {
    return (SplitResult){0, NULL};
  }

  /* Child split occurred; absorb the promoted key */
  if (node->num_keys < TT_MAX_KEYS) {
    /* Make room for the new child pointer */
    if (idx == 0) {
      node->children[2] = node->children[1];
    }
    node->children[idx + 1] = child_result.new_right;
    node_insert_key(node, child_result.middle_key);
    return (SplitResult){0, NULL};
  }

  /* This node is also full; split propagates further up */
  return split_internal(node, child_result.middle_key, child_result.new_right,
                        idx);
}

/* === Delete helpers === */

/**
 * Recursive delete. Returns true if this node ended up empty
 * (underflow), signaling the caller to resolve it.
 */
static bool delete_recursive(Node *node, int key) {
  /* Leaf: remove the key if present */
  if (is_leaf(node)) {
    int idx = find_key_in_node(node, key);
    if (idx < 0)
      return false;

    for (int i = idx; i < node->num_keys - 1; i++) {
      node->keys[i] = node->keys[i + 1];
    }
    node->num_keys--;

    return (node->num_keys == 0);
  }

  /* Internal: if key is found here, swap with inorder successor */
  int key_idx = find_key_in_node(node, key);
  if (key_idx >= 0) {
    Node *successor = find_min_leaf(node->children[key_idx + 1]);
    node->keys[key_idx] = successor->keys[0];

    bool child_underflow =
        delete_recursive(node->children[key_idx + 1], successor->keys[0]);
    if (child_underflow) {
      return resolve_underflow(node, key_idx + 1);
    }
    return false;
  }

  /* Key not in this node; descend into the appropriate child */
  int child_idx = find_child_index(node, key);
  bool child_underflow = delete_recursive(node->children[child_idx], key);
  if (child_underflow) {
    return resolve_underflow(node, child_idx);
  }
  return false;
}

/**
 * Resolve underflow of parent->children[empty_idx] by either redistributing
 * from a sibling (if possible) or merging. Convention: prefer the left
 * sibling.
 *
 * Returns true if the parent itself underflowed as a result (only possible
 * for merge, when the parent was a 2-node).
 */
static bool resolve_underflow(Node *parent, int empty_idx) {
  int sibling_idx = (empty_idx == 0) ? 1 : empty_idx - 1;
  Node *sibling = parent->children[sibling_idx];

  if (sibling->num_keys == TT_MAX_KEYS) {
    redistribute(parent, empty_idx, sibling_idx);
    return false;
  }

  merge(parent, empty_idx, sibling_idx);
  return (parent->num_keys == 0);
}

/**
 * Borrow a key from a 3-node sibling via the parent's separator.
 * Works for both leaf and internal nodes; for internal, also moves
 * the adjacent child pointer.
 *
 * Preconditions: empty has 0 keys, sibling has 2 keys.
 */
static void redistribute(Node *parent, int empty_idx, int sibling_idx) {
  Node *empty = parent->children[empty_idx];
  Node *sibling = parent->children[sibling_idx];

  assert(empty->num_keys == 0);
  assert(sibling->num_keys == TT_MAX_KEYS);

  int sep_idx = (sibling_idx < empty_idx) ? sibling_idx : empty_idx;

  if (sibling_idx < empty_idx) {
    /* Left sibling: borrow the sibling's largest key */
    empty->keys[0] = parent->keys[sep_idx];
    parent->keys[sep_idx] = sibling->keys[1];

    if (!is_leaf(empty)) {
      empty->children[1] = empty->children[0];
      empty->children[0] = sibling->children[2];
    }
  } else {
    /* Right sibling: borrow the sibling's smallest key */
    empty->keys[0] = parent->keys[sep_idx];
    parent->keys[sep_idx] = sibling->keys[0];
    sibling->keys[0] = sibling->keys[1];

    if (!is_leaf(empty)) {
      empty->children[1] = sibling->children[0];
      sibling->children[0] = sibling->children[1];
      sibling->children[1] = sibling->children[2];
    }
  }

  empty->num_keys = 1;
  sibling->num_keys = 1;
}

/**
 * Merge an empty node with a 2-node sibling, pulling down the separator
 * from the parent. The sibling becomes a 3-node holding all keys; the
 * empty node is freed. The parent loses one key and one child.
 *
 * Preconditions: empty has 0 keys, sibling has 1 key.
 */
static void merge(Node *parent, int empty_idx, int sibling_idx) {
  Node *empty = parent->children[empty_idx];
  Node *sibling = parent->children[sibling_idx];

  assert(parent != NULL);
  assert(empty->num_keys == 0);
  assert(sibling->num_keys == 1);

  int sep_idx = (sibling_idx < empty_idx) ? sibling_idx : empty_idx;

  if (sibling_idx < empty_idx) {
    /* Left sibling: append separator (and empty's child) to sibling */
    sibling->keys[1] = parent->keys[sep_idx];
    if (!is_leaf(empty)) {
      sibling->children[2] = empty->children[0];
    }
  } else {
    /* Right sibling: prepend separator (and empty's child) to sibling */
    sibling->keys[1] = sibling->keys[0];
    sibling->keys[0] = parent->keys[sep_idx];
    if (!is_leaf(empty)) {
      sibling->children[2] = sibling->children[1];
      sibling->children[1] = sibling->children[0];
      sibling->children[0] = empty->children[0];
    }
  }
  sibling->num_keys = 2;

  /* Shift parent's keys and children to fill the gap */
  for (int i = sep_idx; i < parent->num_keys - 1; i++) {
    parent->keys[i] = parent->keys[i + 1];
  }
  for (int i = empty_idx; i < parent->num_keys; i++) {
    parent->children[i] = parent->children[i + 1];
  }
  parent->num_keys--;

  free(empty);
}

/* === Public API === */

void tree_insert(Node **root, int key) {
  assert(root != NULL);

  if (*root == NULL) {
    *root = create_node(key);
    return;
  }

  SplitResult r = insert_recursive(*root, key);

  /* Root split: create a new root one level higher */
  if (r.new_right != NULL) {
    Node *new_root = malloc(sizeof(*new_root));
    assert(new_root != NULL);
    new_root->keys[0] = r.middle_key;
    new_root->num_keys = 1;
    new_root->children[0] = *root;
    new_root->children[1] = r.new_right;
    new_root->children[2] = NULL;
    *root = new_root;
  }
}

void tree_delete(Node **root, int key) {
  assert(root != NULL);
  if (*root == NULL)
    return;

  delete_recursive(*root, key);

  /* Root underflow: shrink tree by one level */
  if ((*root)->num_keys == 0) {
    Node *old_root = *root;
    if (is_leaf(*root)) {
      *root = NULL;
    } else {
      *root = (*root)->children[0];
    }
    free(old_root);
  }
}

void tree_destroy(Node **root) {
  assert(root != NULL);
  if (*root == NULL)
    return;

  if (!is_leaf(*root)) {
    for (int i = 0; i <= (*root)->num_keys; i++) {
      tree_destroy(&(*root)->children[i]);
    }
  }

  free(*root);
  *root = NULL;
}

/* === Print (for debugging) === */

static void print_recursive(const Node *node, int depth) {
  if (node == NULL)
    return;

  /* Print rightmost subtree first for sideways layout */
  if (!is_leaf(node)) {
    print_recursive(node->children[node->num_keys], depth + 1);
  }

  for (int i = 0; i < depth; i++)
    printf("    ");
  if (node->num_keys == 1) {
    printf("[%d]\n", node->keys[0]);
  } else {
    printf("[%d, %d]\n", node->keys[0], node->keys[1]);
  }

  if (!is_leaf(node)) {
    if (node->num_keys == 2) {
      print_recursive(node->children[1], depth + 1);
    }
    print_recursive(node->children[0], depth + 1);
  }
}

void tree_print(const Node *root) {
  if (root == NULL) {
    printf("(empty tree)\n");
    return;
  }
  printf("--- 2-3 tree ---\n");
  print_recursive(root, 0);
  printf("----------------\n");
}
