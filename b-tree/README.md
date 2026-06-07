# B-Tree

A self-balancing search tree implemented in C, parameterized by minimum
degree `T`. Supports insert, delete, search, and destroy. Both insert and
delete run **top-down** in a single downward pass.

## Properties

- Parameterized by minimum degree `T` (`>= 2`); the order is `M = 2T`.
- A node holds `T-1 .. 2T-1` keys and, if internal, one more child than keys.
- The root is exempt from the lower bound (it may hold a single key).
- All leaves reside at the same depth (perfectly height-balanced).
- Keys within a node are sorted; the search-tree ordering holds across the
  whole tree.

Using `T` (rather than the order `M`) as the parameter forces an even maximum
number of children, so a split divides a full node symmetrically into
`T-1 | median | T-1`. The two halves always meet the occupancy lower bound,
which keeps the split logic free of the odd/even special cases that arise
when the tree is parameterized by `M` directly.

## Operations

| Operation | Time complexity | Space complexity |
|-----------|-----------------|------------------|
| Search    | O(log n)        | O(log n) stack   |
| Insert    | O(log n)        | O(log n) stack   |
| Delete    | O(log n)        | O(log n) stack   |
| Destroy   | O(n)            | O(log n) stack   |

Height is `O(log_T n)`, so larger `T` means a shallower tree and fewer node
visits — the property that makes B-trees the standard for block-oriented
storage (databases, filesystems), where each node maps to a disk block and a
node visit is one I/O. Within a node the search is linear here; switching to
binary search lowers the per-node comparison count from `O(T)` to `O(log T)`
but does not change the number of node visits.

## Files

| File        | Contents                        |
|-------------|---------------------------------|
| `b_tree.h`  | Public API and `Node` structure |
| `b_tree.c`  | Implementation                  |
| `main.c`    | Demo + randomized stress test   |
| `Makefile`  | Build configuration             |

## Build and run

```bash
make            # build
make run        # build and run the demo + stress test
make stress     # reproducible stress run (fixed seed)
make valgrind   # run under valgrind (requires valgrind)
make asan       # run under AddressSanitizer + UndefinedBehaviorSanitizer
make clean      # remove build artifacts
```

`T` is a compile-time constant in `b_tree.h`; change it and rebuild to test
other branching factors (e.g. `T=2` gives a 2-3-4 tree, where splits are most
frequent and tree growth is easiest to observe).

## Algorithm notes

Both operations rely on the same idea: fix up the child you are about to
descend into *before* descending, so structural changes never have to
propagate back up.

### Insert

Insertion always lands in a leaf. On the way down, any **full** child
(`2T-1` keys) is split first: its median key is promoted into the parent and
its upper half becomes a new sibling. Because the parent was made non-full
before the descent reached it, it always has room for the promoted key, so a
split never cascades upward. The root is the sole exception — when the root
itself is full it is split first, creating a new root. This is the only point
at which tree height increases.

### Delete

Deletion is reduced to deleting from a leaf:

1. **Key in an internal node.** Replace it with its in-order predecessor or
   successor (whichever neighboring child can spare a key), then delete that
   value from the leaf it came from. If neither child can spare a key, merge
   the two children with the separator and recurse into the merged node.
2. **Descending toward a leaf.** Before stepping into a child that is at the
   minimum size (`T-1` keys), refill it so that removing a key cannot
   underflow it:
   - **Borrow** a key from a sibling that has more than the minimum, rotating
     it through the parent's separator. Borrowing does not change the
     parent's key count, so it never propagates.
   - **Merge** with a sibling when neither neighbor can spare a key. A merge
     pulls the separator down and removes one key from the parent, which may
     in turn require a fixup one level up — already handled, because the
     parent was refilled on the way down.
3. If the root ends up empty (its last key was pulled into a merge), it is
   removed and its sole child becomes the new root. This is the only point at
   which tree height decreases.

### Borrow vs. merge

Borrowing is preferred because it leaves the parent's occupancy unchanged and
therefore cannot trigger further fixups. Merging is the fallback for when both
siblings are already at the minimum; it is always valid there because
`(T-1) + 1 + (T-1) = 2T-1`, exactly a full node.

## Testing

`main.c` runs two layers:

- A scripted demo that prints the tree (sideways, rotated 90°) through a
  sequence of inserts and deletes.
- A randomized stress test (200k operations by default) that mirrors every
  operation against a reference set and re-validates the tree against all
  B-tree invariants. The validator checks occupancy bounds, per-node sort
  order, the global key-range ordering, and equal leaf depth, so any
  structural bug fails an assertion at the exact operation that introduced it.

Verified clean under `valgrind` (0 errors, 0 leaks), AddressSanitizer, and
UndefinedBehaviorSanitizer, at `-O0` and `-O2`.

## References

- CLRS, *Introduction to Algorithms*, Chapter 18 (B-Trees) — the top-down
  insert/delete formulation followed here.
- Bayer & McCreight (1972), *Organization and Maintenance of Large Ordered
  Indexes* — the original B-tree paper.
