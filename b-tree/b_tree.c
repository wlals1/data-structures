/*
 * b_tree.c -- B-tree implementation (see b_tree.h for the contract).
 *
 * Both insert and delete are top-down: each prepares the child it is about
 * to descend into so that structural fixups never have to propagate back up.
 *   - insert_nonfull splits a full child before descending.
 *   - delete_recursive refills a minimum-size child before descending.
 */

#include "b_tree.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/* ====================================================================== */
/* Node helpers                                                           */
/* ====================================================================== */

/* Allocate a zero-initialized node (num_keys == 0, all children NULL).
 * The caller is responsible for populating keys/children. */
static Node *create_node(void) {
  Node *node = calloc(1, sizeof(Node));
  assert(node);
  return node;
}

/* A node is a leaf iff it has no children. create_node zero-fills the
 * children array, so children[0] == NULL is a reliable test. */
static bool is_leaf(const Node *n) { return n->children[0] == NULL; }

/* Largest key in the subtree: walk to the rightmost leaf. */
static int subtree_max(const Node *node) {
  assert(node != NULL);
  while (!is_leaf(node))
    node = node->children[node->num_keys];
  return node->keys[node->num_keys - 1];
}

/* Smallest key in the subtree: walk to the leftmost leaf. */
static int subtree_min(const Node *node) {
  assert(node != NULL);
  while (!is_leaf(node))
    node = node->children[0];
  return node->keys[0];
}

/* ====================================================================== */
/* Search                                                                 */
/* ====================================================================== */

bool tree_search(const Node *root, int key) {
  if (root == NULL)
    return false;

  int i = 0;
  while (i < root->num_keys && key > root->keys[i])
    i++;

  if (i < root->num_keys && key == root->keys[i])
    return true;

  /* key < keys[i] (or i == num_keys): descend into children[i].
   * For a leaf, children[i] is NULL and the NULL guard returns false. */
  return tree_search(root->children[i], key);
}

/* ====================================================================== */
/* Insert (top-down: split full children on the way down)                 */
/* ====================================================================== */

/* Split parent->children[i], which must be full (MAX_KEYS keys).
 * The median key (index T-1) is promoted into parent at index i, and the
 * upper half becomes a new sibling inserted at parent->children[i+1].
 * parent must not be full, so it always has room for the promoted key. */
static void split_child(Node *parent, int i) {
  Node *y = parent->children[i];
  assert(y->num_keys == MAX_KEYS);

  Node *z = create_node();
  z->num_keys = T - 1;

  /* z takes y's upper half: keys[T .. 2T-2]. */
  for (int j = 0; j < T - 1; j++)
    z->keys[j] = y->keys[T + j];

  /* and the matching children if y is internal: children[T .. 2T-1]. */
  if (!is_leaf(y))
    for (int j = 0; j < T; j++)
      z->children[j] = y->children[T + j];

  /* y keeps its lower half; the tail is now logically gone. */
  y->num_keys = T - 1;

  /* Open a slot at keys[i] for the median, shifting right. */
  for (int j = parent->num_keys - 1; j >= i; j--)
    parent->keys[j + 1] = parent->keys[j];
  parent->keys[i] = y->keys[T - 1];

  /* Open a slot at children[i+1] for z, shifting right. */
  for (int j = parent->num_keys; j >= i + 1; j--)
    parent->children[j + 1] = parent->children[j];
  parent->children[i + 1] = z;

  parent->num_keys++;
}

/* Insert key into x, which is guaranteed not full. */
static void insert_nonfull(Node *x, int key) {
  if (is_leaf(x)) {
    /* Shift larger keys right and drop key into the gap. */
    int i = x->num_keys - 1;
    while (i >= 0 && key < x->keys[i]) {
      x->keys[i + 1] = x->keys[i];
      i--;
    }
    x->keys[i + 1] = key;
    x->num_keys++;
    return;
  }

  /* Find the child to descend into. */
  int i = 0;
  while (i < x->num_keys && key > x->keys[i])
    i++;

  /* If it is full, split it first; the median may shift our target. */
  if (x->children[i]->num_keys == MAX_KEYS) {
    split_child(x, i);
    if (key > x->keys[i])
      i++;
  }

  insert_nonfull(x->children[i], key);
}

void tree_insert(Node **root, int key) {
  if (*root == NULL) {
    *root = create_node();
    (*root)->keys[0] = key;
    (*root)->num_keys = 1;
    return;
  }

  /* If the root is full, grow upward first: the only place tree height
   * ever increases. */
  if ((*root)->num_keys == MAX_KEYS) {
    Node *s = create_node();
    s->children[0] = *root;
    *root = s;
    split_child(s, 0);
  }

  insert_nonfull(*root, key);
}

/* ====================================================================== */
/* Delete (top-down: refill minimum-size children on the way down)        */
/* ====================================================================== */

/* Borrow one key from the left sibling, rotating through the parent:
 * parent->keys[i-1] moves down to the front of children[i], and the
 * sibling's largest key moves up into parent->keys[i-1]. If internal, the
 * sibling's rightmost child moves to the front of children[i] as well. */
static void borrow_from_prev(Node *parent, int i) {
  Node *child = parent->children[i];
  Node *sibling = parent->children[i - 1];

  for (int j = child->num_keys - 1; j >= 0; j--)
    child->keys[j + 1] = child->keys[j];

  if (!is_leaf(child)) {
    for (int j = child->num_keys; j >= 0; j--)
      child->children[j + 1] = child->children[j];
    child->children[0] = sibling->children[sibling->num_keys];
  }

  child->keys[0] = parent->keys[i - 1];
  parent->keys[i - 1] = sibling->keys[sibling->num_keys - 1];

  child->num_keys++;
  sibling->num_keys--;
}

/* Mirror of borrow_from_prev: borrow from the right sibling.
 * parent->keys[i] moves down to the back of children[i], the sibling's
 * smallest key moves up into parent->keys[i], and (if internal) the
 * sibling's leftmost child moves to the back of children[i]. */
static void borrow_from_next(Node *parent, int i) {
  Node *child = parent->children[i];
  Node *sibling = parent->children[i + 1];

  child->keys[child->num_keys] = parent->keys[i];
  parent->keys[i] = sibling->keys[0];

  for (int j = 1; j < sibling->num_keys; j++)
    sibling->keys[j - 1] = sibling->keys[j];

  child->num_keys++;

  if (!is_leaf(child)) {
    child->children[child->num_keys] = sibling->children[0];
    for (int j = 1; j < sibling->num_keys + 1; j++)
      sibling->children[j - 1] = sibling->children[j];
  }

  sibling->num_keys--;
}

/* Merge children[i], the separator parent->keys[i], and children[i+1] into
 * a single node (children[i]). The result has (T-1)+1+(T-1) = 2T-1 keys,
 * exactly full. parent loses one key and one child; children[i+1] is freed. */
static void merge(Node *parent, int i) {
  Node *child = parent->children[i];
  Node *sibling = parent->children[i + 1];

  /* Pull the separator down, then append the sibling's keys. */
  child->keys[child->num_keys] = parent->keys[i];
  for (int j = 0; j < sibling->num_keys; j++)
    child->keys[child->num_keys + 1 + j] = sibling->keys[j];

  /* Append the sibling's children too (offset by the separator slot). */
  if (!is_leaf(child))
    for (int j = 0; j < sibling->num_keys + 1; j++)
      child->children[child->num_keys + 1 + j] = sibling->children[j];

  child->num_keys += 1 + sibling->num_keys;

  /* Close the gap left in the parent (shift left). */
  for (int j = i; j < parent->num_keys - 1; j++)
    parent->keys[j] = parent->keys[j + 1];
  for (int j = i + 1; j < parent->num_keys; j++)
    parent->children[j] = parent->children[j + 1];
  parent->num_keys--;

  free(sibling);
}

/* Ensure children[i] has more than MIN_KEYS keys before we descend into it.
 * Prefer borrowing (which does not shrink the parent); fall back to merging
 * with a sibling when neither neighbor can spare a key. */
static void fill(Node *x, int i) {
  if (i > 0 && x->children[i - 1]->num_keys > MIN_KEYS)
    borrow_from_prev(x, i);
  else if (i < x->num_keys && x->children[i + 1]->num_keys > MIN_KEYS)
    borrow_from_next(x, i);
  else if (i < x->num_keys)
    merge(x, i); /* merge with the right sibling */
  else
    merge(x, i - 1); /* last child: merge with the left sibling */
}

static void delete_recursive(Node *x, int key) {
  int idx = 0;
  while (idx < x->num_keys && key > x->keys[idx])
    idx++;

  /* Case 1 & 2: the key lives in this node. */
  if (idx < x->num_keys && x->keys[idx] == key) {
    if (is_leaf(x)) {
      /* Case 1: remove from a leaf (guaranteed >= MIN_KEYS+1 keys). */
      for (int j = idx; j < x->num_keys - 1; j++)
        x->keys[j] = x->keys[j + 1];
      x->num_keys--;
    } else if (x->children[idx]->num_keys > MIN_KEYS) {
      /* Case 2a: left child can spare a key -> replace with predecessor. */
      int pred = subtree_max(x->children[idx]);
      x->keys[idx] = pred;
      delete_recursive(x->children[idx], pred);
    } else if (x->children[idx + 1]->num_keys > MIN_KEYS) {
      /* Case 2b: right child can spare a key -> replace with successor. */
      int succ = subtree_min(x->children[idx + 1]);
      x->keys[idx] = succ;
      delete_recursive(x->children[idx + 1], succ);
    } else {
      /* Case 2c: both children are minimal -> merge and delete below. */
      merge(x, idx);
      delete_recursive(x->children[idx], key);
    }
    return;
  }

  /* Case 3: the key is not here; descend into children[idx]. */
  if (is_leaf(x))
    return; /* a leaf without the key means it is not in the tree */

  /* Refill before descending so the recursion's precondition holds. */
  if (x->children[idx]->num_keys == MIN_KEYS)
    fill(x, idx);

  /* A merge of the last child folds children[idx] into children[idx-1] and
   * shrinks num_keys, so idx may now point one past the end. */
  if (idx > x->num_keys)
    delete_recursive(x->children[idx - 1], key);
  else
    delete_recursive(x->children[idx], key);
}

void tree_delete(Node **root, int key) {
  if (*root == NULL)
    return;

  delete_recursive(*root, key);

  /* If the root was emptied (its last key was pulled into a merge), drop it:
   * the sole remaining child becomes the new root. The only place tree
   * height ever decreases. */
  if ((*root)->num_keys == 0) {
    Node *old = *root;
    *root = is_leaf(*root) ? NULL : (*root)->children[0];
    free(old);
  }
}

/* ====================================================================== */
/* Destroy                                                                */
/* ====================================================================== */

void tree_destroy(Node **root) {
  assert(root != NULL);
  if (*root == NULL)
    return;

  if (!is_leaf(*root))
    for (int i = 0; i <= (*root)->num_keys; i++)
      tree_destroy(&(*root)->children[i]);

  free(*root);
  *root = NULL;
}

/* ====================================================================== */
/* Print (sideways, for debugging)                                        */
/* ====================================================================== */

/* In-order walk that prints the rightmost subtree first, so the tree shows
 * up rotated 90 degrees counter-clockwise with the largest key on top. */
static void print_recursive(const Node *node, int depth) {
  if (node == NULL)
    return;

  print_recursive(node->children[node->num_keys], depth + 1);

  for (int i = 0; i < depth; i++)
    printf("    ");
  for (int i = 0; i < node->num_keys; i++)
    printf("%d ", node->keys[i]);
  printf("\n");

  for (int i = node->num_keys - 1; i >= 0; i--)
    print_recursive(node->children[i], depth + 1);
}

void tree_print(const Node *root) {
  if (root == NULL) {
    printf("(empty tree)\n");
    return;
  }
  print_recursive(root, 0);
}
