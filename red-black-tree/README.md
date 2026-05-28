# Red-Black Tree

A self-balancing binary search tree implemented in C, supporting insert,
delete, search, and destroy operations.

## Properties

- BST property: left subtree keys < node key < right subtree keys.
- Every node is red or black.
- The root is black, and every NIL leaf is black.
- A red node has no red child (no two reds in a row).
- Every root-to-NIL path through a node has the same number of black
  nodes (its black-height).

These invariants force the longest root-to-leaf path to be at most twice
the shortest, so the height stays `O(log n)`.

## Operations

| Operation | Time complexity | Space complexity |
|-----------|-----------------|------------------|
| Search    | O(log n)        | O(1)             |
| Insert    | O(log n)        | O(1)             |
| Delete    | O(log n)        | O(1)             |
| Destroy   | O(n)            | O(log n) stack   |

Insert and delete run in `O(log n)` time but use only `O(1)` extra space:
they are iterative and rely on parent pointers rather than recursion.

## Files

| File         | Contents                            |
|--------------|-------------------------------------|
| `rb_tree.h`  | Public API and `Node` structure     |
| `rb_tree.c`  | Implementation                      |
| `main.c`     | Unit tests and randomized stress tests |
| `Makefile`   | Build configuration                 |

## Build and run

```bash
make            # build
make run        # build and run tests
make valgrind   # run tests under valgrind (requires valgrind)
make clean      # remove build artifacts
```

## API style

This implementation uses **pattern B** (the root is passed by address
and updated in place), unlike the AVL tree in this repository, which
uses pattern A (return value). Pattern B is the natural fit here because
nodes carry parent pointers, so the functions can relink the entire tree
themselves and the caller never reassigns the root.

```c
Node *root = NIL;          /* an empty tree is NIL, not NULL */
rb_insert(&root, 50);
rb_insert(&root, 30);
rb_delete(&root, 50);
rb_destroy(&root);
```

### The NIL sentinel

Every empty position — including an empty tree — is the single shared
sentinel `NIL`, which is always black. Using a real node instead of
`NULL` lets the rotation and fixup code read `NIL->color` and set
`NIL->parent` without special-casing null pointers, which removes a lot
of branching from the delete path in particular. Callers must initialize
an empty tree with `Node *root = NIL;` and may compare against `NIL`.

## Algorithm notes

### Rotations

`rotate_left` and `rotate_right` are pure structural operations that do
not touch colors; the caller adjusts colors around them. Both reuse a
small helper, `replace_child`, for the "repoint the parent's child link"
step — the same three-way branch (`root` / left child / right child)
that `transplant` uses. The Linux kernel factors out the equivalent
helper as `__rb_change_child`.

### Insert

A new node is inserted as in a plain BST and colored **red**. Coloring it
red means the only invariant it can break is the no-red-red rule
(invariant 4), which is repaired locally; coloring it black would instead
break the equal-black-height rule (invariant 5), which cannot be repaired
locally. `insert_fixup` then walks up from the new node:

| Case | Trigger                          | Action                              |
|------|----------------------------------|-------------------------------------|
| 1    | uncle is red                     | recolor, continue two levels up     |
| 2    | uncle black, node is a bent child| rotate at parent → becomes Case 3   |
| 3    | uncle black, node is straight    | recolor and rotate at grandparent   |

Case 1 only recolors and may repeat up to the root, so recoloring can
happen `O(log n)` times. A rotation only occurs in Case 2/3, both of
which end the loop, so **an insert performs at most two rotations**.

### Delete

A delete proceeds in two phases:

1. **BST delete** (`rb_delete`): remove the target node. Cases:
   - No children or one child: splice the node out with `transplant`.
   - Two children: replace the node with its in-order successor (which
     has at most one child), moving the successor node — not just its
     key — so that color bookkeeping stays correct.
   The color of the node actually spliced out is remembered.
2. **Fixup** (`delete_fixup`): if the spliced-out node was **black**, a
   path is now one black short. The node filling that gap carries an
   "extra black" (it is conceptually doubly black), and the fixup pushes
   that extra black around until it is absorbed. With `w` as the sibling:

| Case | Trigger                                   | Action                          |
|------|-------------------------------------------|---------------------------------|
| 1    | `w` red                                   | recolor and rotate → sibling becomes black |
| 2    | `w` black, both `w`'s children black      | recolor `w` red, push extra black up to parent |
| 3    | `w` black, near child red, far child black| recolor and rotate at `w` → becomes Case 4 |
| 4    | `w` black, far child red                  | recolor and rotate at parent; done |

Only Case 2 propagates (no rotation); the worst rotating path is
1 → 3 → 4, so **a delete performs at most three rotations** — in contrast
to an AVL delete, which can rotate `O(log n)` times.

## Testing

`main.c` runs functional unit tests (insert/search/delete, duplicate
inserts, deleting absent keys, deleting a tree down to empty) and two
randomized stress tests that perform thousands of operations, calling
`rb_is_valid` after every single one. `rb_is_valid` checks all four
red-black invariants plus BST ordering. The suite is clean under
valgrind (no leaks, no errors) and under AddressSanitizer/UBSan.

## References

- CLRS, *Introduction to Algorithms*, Chapter 13 (Red-Black Trees)
- Linux kernel `lib/rbtree.c` — the same algorithm tuned for production
  (NULL instead of a sentinel, color stored in the low bits of the
  parent pointer).
