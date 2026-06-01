# k-d Tree

A 2-dimensional k-d tree in C, supporting insertion and **nearest-neighbor
(NN) search** with branch-and-bound pruning. A k-d tree partitions space by
cycling through the coordinate axes at successive depths, which makes spatial
queries (nearest neighbor, range search) fast in low dimensions.

## Structure

Each node stores a point and has two children. Unlike a 1-D BST, there is no
single ordering on multidimensional points, so the tree compares **one axis
per level**, cycling through them by depth:

- depth 0, 2, 4, … compare the x-coordinate (`axis = 0`)
- depth 1, 3, 5, … compare the y-coordinate (`axis = 1`)
- in general `axis = depth % K`

A node at depth `d` splits its region with a line at its own coordinate on
that axis — a vertical line `x = node.x` for an x-node, a horizontal line
`y = node.y` for a y-node. The axis is **not stored** in the node; it is
derived from the recursion depth, which is passed as an argument.

```c
#define K 2
typedef struct kdnode {
    double point[K];
    struct kdnode *left, *right;
} kdnode;
```

The node definition is exposed in the header (not opaque), because the
caller reads the coordinates of the point returned by a query — this is the
common convention for geometric structures.

## Nearest-neighbor search

Descending to a leaf like a BST does **not** suffice: the descent direction
is decided by which side of a split line the query falls on, which is not the
same question as which side holds the closest point. A closer point can sit
just across a split line, in the subtree the descent never visited.

So NN search is descend-then-backtrack with pruning:

1. **Descend.** At each node, compare the query against the node's point on
   the current axis, recursing into the *near* child (the side the query
   belongs to) first. Measure the distance to every node visited (every node
   is a point) and keep the best so far.
2. **Backtrack and prune.** Returning from the near child, decide whether the
   *far* child can hold anything closer. Picture a circle centered at the
   query with radius equal to the current best distance. If that circle
   crosses the node's split line, the far side might contain a closer point —
   recurse into it. If it does not, skip the entire far subtree.

The pruning test compares the **distance from the query to the split line**
(an axis-only distance, `|query[axis] - node[axis]|`) against the **current
best distance**. Searching the near child first shrinks the best distance
early, so more far subtrees get pruned later.

### Squared distances

All comparisons use **squared** distance, never `sqrt`. The square root is
monotonic, so it does not change which point is closer, and skipping it
avoids a costly call at every node. The pruning test squares the axis
distance too, so both sides of the comparison are in the same units
(distance²): `(query[axis] - node[axis])² < best_sq`.

## Why descend-first matters

The near child is searched before the far child is even *considered*. This is
not cosmetic: a good candidate found early shrinks the search radius, which
makes the `axis_dist² < best_sq` test fail more often, pruning more of the
remaining tree. Reverse the order and pruning becomes far less effective.

## Complexity

`n` = number of points, `K` = dimensions.

| Operation         | Average     | Worst case |
|-------------------|-------------|------------|
| Insert            | O(log n)    | O(n)       |
| Nearest neighbor  | O(log n)    | O(n)       |

Both degrade to O(n) on an unbalanced tree (e.g. points inserted in sorted
order) or when pruning fails. Pruning loses effectiveness as `K` grows — the
**curse of dimensionality** — so k-d trees are a good fit only in low
dimensions (roughly K up to ~20). High-dimensional nearest-neighbor search
uses approximate methods (e.g. HNSW) instead.

## API

```c
kdnode *kd_insert(kdnode *root, const double point[K]);  /* returns new root */
kdnode *kd_nearest(kdnode *root, const double query[K]);  /* NULL if empty */
void    kd_free(kdnode *root);
```

`kd_insert` follows the recursive BST pattern: it returns the (possibly new)
subtree root, which the caller assigns back — `root = kd_insert(root, pt)`.

## Testing

`main.c` validates NN search against brute force: it scatters random points,
runs thousands of random queries, and checks that the k-d result matches the
exhaustive O(n) scan on every query. The comparison is by **distance**, not
node identity, because ties (two points equidistant from the query) are both
correct answers. A computed distance function independent of the library's
own is used, so a bug in one would not be masked by the same bug in the
other.

## Files

| File         | Contents                                          |
|--------------|---------------------------------------------------|
| `kd_tree.h`  | `kdnode`, `K`, public API                         |
| `kd_tree.c`  | Insert, nearest-neighbor search, pruning, free    |
| `main.c`     | Randomized test against brute-force ground truth  |
| `Makefile`   | Build configuration                               |

## Build and run

```bash
make            # build
make run        # build and run tests
make valgrind   # run under valgrind
make clean      # remove build artifacts
```

## References

- J. L. Bentley, "Multidimensional Binary Search Trees Used for Associative
  Searching," *Communications of the ACM*, 1975 (the original k-d tree paper)
- de Berg et al., *Computational Geometry: Algorithms and Applications*,
  Chapter 5 (orthogonal range searching, k-d trees)
