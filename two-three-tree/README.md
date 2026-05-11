# 2-3 Tree

A self-balancing search tree implemented in C, supporting insert, delete,
search, and destroy operations.

## Properties

- Each internal node has either 2 or 3 children (2-node or 3-node).
- Number of children equals number of keys + 1.
- All leaves reside at the same depth (perfectly height-balanced).
- Keys within a node are sorted; the BST property holds across the tree.

## Operations

| Operation | Time complexity | Space complexity |
|-----------|----------------|------------------|
| Search    | O(log n)       | O(log n) stack   |
| Insert    | O(log n)       | O(log n) stack   |
| Delete    | O(log n)       | O(log n) stack   |
| Destroy   | O(n)           | O(log n) stack   |

## Files

| File                 | Contents                            |
|----------------------|-------------------------------------|
| `two_three_tree.h`   | Public API and `Node` structure     |
| `two_three_tree.c`   | Implementation                      |
| `main.c`             | Unit tests                          |
| `Makefile`           | Build configuration                 |

## Build and run

```bash
make            # build
make run        # build and run tests
make valgrind   # run tests under valgrind (requires valgrind)
make clean      # remove build artifacts
```

## Algorithm notes

### Insert

Inserts always happen at a leaf. When a leaf overflows (3 keys), it is
split: the middle key is promoted to the parent, and a new sibling holds
the largest key. Splits can cascade up to the root, increasing tree height
by 1.

### Delete

Deletes are reduced to leaf deletions:

1. If the key is in an internal node, swap it with its inorder successor
   (the smallest key in the right subtree's leftmost leaf), then delete
   the successor from the leaf.
2. After removing the key from a leaf, if the leaf becomes empty
   (underflow), resolve via one of:
   - **Redistribute**: if a sibling has 2 keys, borrow one through the
     parent's separator.
   - **Merge**: otherwise, combine the empty node, the parent's separator,
     and the sibling into a single 3-node. This may propagate underflow
     to the parent.
3. If the root becomes empty, the tree's height decreases by 1.

### Underflow handling

The convention used here prefers the left sibling when both are available
(only the case for the middle child of a 3-node parent). A more aggressive
policy could check both siblings for redistribution before falling back to
merge; this is left as a future improvement.

## References

- Sedgewick, *Algorithms*, Chapter 6.2 (Balanced Search Trees)
- CLRS, *Introduction to Algorithms*, Chapter 18 (B-Trees)
  — generalizes the same ideas to higher branching factors.
