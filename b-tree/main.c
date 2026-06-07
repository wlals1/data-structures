/*
 * main.c -- tests for the B-tree.
 *
 * Two layers:
 *   1. A small scripted demo that prints the tree through a sequence of
 *      inserts and deletes (visual sanity check).
 *   2. A randomized stress test that cross-checks the tree against a
 *      reference set and a structural validator over many operations.
 *
 * The validator checks every B-tree invariant, so any structural bug
 * surfaces as a failed assertion at the exact operation that caused it.
 */

#include "b_tree.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ====================================================================== */
/* Validator: walk the tree and assert every invariant                    */
/* ====================================================================== */

static bool node_is_leaf(const Node *n) { return n->children[0] == NULL; }

/*
 * Recursively validate the subtree rooted at node.
 *   - is_root: relaxes the MIN_KEYS lower bound for the root.
 *   - lo/hi:   open-interval (lo, hi) every key here must fall in; NULL means
 *              unbounded on that side (carried down from separators).
 * Returns the leaf depth of this subtree; the caller asserts all subtrees
 * agree, which is how "all leaves at the same depth" is checked.
 */
static int validate(const Node *node, bool is_root, const int *lo,
                    const int *hi) {
  assert(node != NULL);

  /* Occupancy bounds. */
  assert(node->num_keys <= MAX_KEYS);
  if (!is_root)
    assert(node->num_keys >= MIN_KEYS);
  else
    assert(node->num_keys >= 1); /* a live root holds at least one key */

  /* Keys sorted ascending and within (lo, hi). */
  for (int i = 0; i < node->num_keys; i++) {
    if (i > 0)
      assert(node->keys[i - 1] < node->keys[i]);
    if (lo)
      assert(node->keys[i] > *lo);
    if (hi)
      assert(node->keys[i] < *hi);
  }

  if (node_is_leaf(node))
    return 0;

  /* Internal node: recurse into each child with tightened bounds, and
   * require all children to report the same depth. */
  int child_depth = -1;
  for (int i = 0; i <= node->num_keys; i++) {
    const int *child_lo = (i == 0) ? lo : &node->keys[i - 1];
    const int *child_hi = (i == node->num_keys) ? hi : &node->keys[i];
    int d = validate(node->children[i], false, child_lo, child_hi);
    if (child_depth == -1)
      child_depth = d;
    else
      assert(d == child_depth);
  }
  return child_depth + 1;
}

static void check_tree(const Node *root) {
  if (root != NULL)
    validate(root, true, NULL, NULL);
}

/* ====================================================================== */
/* Scripted demo                                                          */
/* ====================================================================== */

static void demo(void) {
  printf("########## scripted demo ##########\n\n");

  Node *root = NULL;

  printf("--- insert 1..20 ---\n");
  for (int i = 1; i <= 20; i++) {
    tree_insert(&root, i);
    check_tree(root);
  }
  tree_print(root);

  int del[] = {1, 10, 20, 6, 15, 2, 3};
  int ndel = (int)(sizeof del / sizeof del[0]);
  for (int k = 0; k < ndel; k++) {
    tree_delete(&root, del[k]);
    check_tree(root);
    printf("\n--- delete %d ---\n", del[k]);
    tree_print(root);
    assert(!tree_search(root, del[k]));
  }

  printf("\n--- delete everything ---\n");
  for (int i = 1; i <= 20; i++)
    tree_delete(&root, i);
  check_tree(root);
  tree_print(root);

  tree_destroy(&root);
  printf("\ndemo passed.\n\n");
}

/* ====================================================================== */
/* Randomized stress test                                                 */
/* ====================================================================== */

#define UNIVERSE 2000 /* key space [0, UNIVERSE)            */
#define OPS 200000    /* number of randomized operations    */

static void stress(unsigned seed) {
  printf("########## stress test ##########\n");
  printf("seed=%u  universe=%d  ops=%d\n", seed, UNIVERSE, OPS);
  srand(seed);

  Node *root = NULL;
  /* Reference set: present[k] == true iff k is in the tree. */
  bool *present = calloc(UNIVERSE, sizeof(bool));
  assert(present);

  for (int op = 0; op < OPS; op++) {
    int key = rand() % UNIVERSE;

    if (rand() & 1) {
      /* Insert, but only if absent (the tree allows duplicates; the
       * reference model does not, so we keep them in lockstep). */
      if (!present[key]) {
        tree_insert(&root, key);
        present[key] = true;
      }
    } else {
      tree_delete(&root, key);
      present[key] = false;
    }

    /* Spot-check membership of the touched key every operation; full
     * structural validation periodically (it is O(n)). */
    assert(tree_search(root, key) == present[key]);
    if ((op & 0x3FF) == 0)
      check_tree(root);
  }

  /* Final pass: structure is valid and membership matches exactly. */
  check_tree(root);
  for (int k = 0; k < UNIVERSE; k++)
    assert(tree_search(root, k) == present[k]);

  /* Drain everything and confirm the tree empties cleanly. */
  for (int k = 0; k < UNIVERSE; k++)
    if (present[k])
      tree_delete(&root, k);
  check_tree(root);
  assert(root == NULL);

  free(present);
  tree_destroy(&root);
  printf("stress test passed.\n");
}

/* ====================================================================== */

int main(int argc, char **argv) {
  demo();

  /* Allow a fixed seed for reproducible runs: ./btree <seed> */
  unsigned seed = (argc > 1) ? (unsigned)strtoul(argv[1], NULL, 10) : 12345u;
  stress(seed);

  return 0;
}
