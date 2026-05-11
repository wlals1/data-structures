# AVL Tree

A self-balancing binary search tree implemented in C, supporting insert,
delete, search, and destroy operations.

## Properties

- BST property: left subtree keys < node key < right subtree keys.
- Balance factor (left height - right height) is in {-1, 0, 1} for every node.
- Height convention: leaf height = 1, NULL height = 0.

## Operations

| Operation | Time complexity | Space complexity |
|-----------|----------------|------------------|
| Search    | O(log n)       | O(log n) stack   |
| Insert    | O(log n)       | O(log n) stack   |
| Delete    | O(log n)       | O(log n) stack   |
| Destroy   | O(n)           | O(log n) stack   |

## Files

| File          | Contents                            |
|---------------|-------------------------------------|
| `avl_tree.h`  | Public API and `Node` structure     |
| `avl_tree.c`  | Implementation                      |
| `main.c`      | Unit tests                          |
| `Makefile`    | Build configuration                 |

## Build and run

```bash
make            # build
make run        # build and run tests
make valgrind   # run tests under valgrind (requires valgrind)
make clean      # remove build artifacts
```

## API style

This implementation uses **pattern A** (return value updates root). The
caller must reassign the root pointer on each operation:

```c
Node *root = NULL;
root = avl_insert(root, 50);
root = avl_insert(root, 30);
root = avl_delete(root, 50);
avl_destroy(root);
```

## Algorithm notes

### Rebalancing after insert/delete

After every insert or delete, the affected path is walked back up to the
root. At each node, the height is recomputed and the balance factor is
checked. If the balance factor becomes ±2, one of four rotation cases
applies:

| Case | Trigger                                | Rotation                          |
|------|----------------------------------------|-----------------------------------|
| LL   | left-heavy, left child is left-heavy   | rotate right                      |
| LR   | left-heavy, left child is right-heavy  | rotate left on left, then right   |
| RR   | right-heavy, right child is right-heavy| rotate left                       |
| RL   | right-heavy, right child is left-heavy | rotate right on right, then left  |

### Delete

A delete proceeds in two phases:

1. **BST delete**: find the target node and remove it. Cases:
   - Leaf: free it directly.
   - One child: replace with the child.
   - Two children: copy the inorder successor's key into the node,
     then recursively delete the successor (which has at most one child).
2. **Rebalance**: as the recursion unwinds, each ancestor is rebalanced.

A single delete can trigger up to O(log n) rotations as the height
change propagates upward, unlike insert which needs at most one.

## References

- Sedgewick, *Algorithms*, Chapter 3.3 (Balanced Search Trees)
- CLRS, *Introduction to Algorithms*, Chapter 13 (Red-Black Trees)
  — a related self-balancing scheme with weaker constraints.
