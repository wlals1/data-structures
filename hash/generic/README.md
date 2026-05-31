# Hash Table (generic)

Two hash table implementations behind a single interface, written in C:
**open addressing** (double hashing) and **separate chaining**. Both support
insert, search, delete, and destroy. The point of this folder is the
contrast: identical API, different internals.

## Shared interface

Both implementations satisfy the same `hash.h`. The node/slot type is
private to each `.c`; callers only ever hold an opaque `HashTable *`.

```c
typedef struct HashTable HashTable;

HashTable *ht_create(void);
bool ht_insert(HashTable *ht, int key, int value);
bool ht_search(const HashTable *ht, int key, int *out_value);
bool ht_delete(HashTable *ht, int key);
void ht_destroy(HashTable *ht);
```

Search contract: `ht_search` returns `true` and writes the value to
`*out_value` when the key is found. When it returns `false`, `*out_value`
is left untouched — read it only when the call returns `true`.

Because `main.c` depends only on `hash.h`, the exact same test program runs
against either implementation; only the linked object file differs.

## The two implementations

| Aspect            | Open addressing            | Separate chaining          |
|-------------------|----------------------------|----------------------------|
| Collision handling| Probe other slots          | Linked list per bucket     |
| Storage           | Fixed array in the struct  | Array of list heads        |
| Deletion          | Mark slot `DELETED` (tombstone) | Unlink node and free   |
| `ht_search` miss  | Stops at first `EMPTY` slot| Reaches end of bucket list |
| `ht_destroy`      | `free(ht)` only            | Free every node, then `free(ht)` |
| Cache behavior    | Contiguous, cache-friendly | Pointer-chasing            |
| Extra memory      | None per entry             | One `next` pointer per node|

### Open addressing (double hashing)

A key is placed by probing the sequence `(h1(k) + i*h2(k)) mod M` for
`i = 0, 1, 2, …`. Using a second hash for the step (rather than `+1`)
spreads probes out and reduces clustering.

- `h1(k) = k mod M`, `h2(k) = R - (k mod R)`. `h2` never returns 0, so the
  step is never zero (a zero step would probe the same slot forever).
- Each slot carries a state: `EMPTY`, `OCCUPIED`, or `DELETED`.
- **Deletion uses a tombstone** (`DELETED`) rather than `EMPTY`. Marking a
  slot `EMPTY` would cut the probe chain, making later keys on that chain
  unreachable. Search treats `DELETED` as "keep probing"; only `EMPTY`
  ends the search.
- **Insert reuses tombstones.** While probing, the first `DELETED` slot is
  remembered but probing continues — so a key already present further along
  is still found and updated rather than duplicated. If the key is absent,
  the remembered tombstone (or the first `EMPTY`) receives the new entry.

### Separate chaining

Each bucket is the head of a singly linked list. Collisions extend the
list. Insertion is at the head (O(1)); duplicate keys are updated in place.

- Deletion unlinks the node and frees it — no tombstones needed, since
  removing a node never breaks the chain.
- `ht_destroy` walks every bucket list, freeing each node (saving the
  `next` pointer before `free` to avoid use-after-free), then frees the
  table struct itself.

## Complexity

`n` = number of entries, `M` = table size, load factor α = n/M.

| Operation | Average      | Worst case |
|-----------|--------------|------------|
| Insert    | O(1)         | O(n)       |
| Search    | O(1)         | O(n)       |
| Delete    | O(1)         | O(n)       |

Both are O(1) on average when α is kept low. They degrade as α rises —
open addressing sharply as α approaches 1 (probe sequences lengthen and
tombstones accumulate), chaining more gracefully (bucket lists grow). The
fixed `M = 13` here means α is not bounded; a production table would
rehash (grow and re-insert) once α crosses a threshold such as 0.75.

`M` is prime so that `k mod M` mixes all bits of the key; a power-of-two
`M` would only use the low bits and cluster more.

## Files

| File               | Contents                                  |
|--------------------|-------------------------------------------|
| `hash.h`           | Shared opaque API                         |
| `hash_open.c`      | Open addressing (double hashing)          |
| `hash_chaining.c`  | Separate chaining                         |
| `main.c`           | Unit tests (run against either backend)   |
| `Makefile`         | Build configuration                       |

## Build and run

Each implementation builds into its own executable, sharing `main.o`.

```bash
make                    # build both: hash_open and hash_chaining
make run-open           # build and run the open-addressing tests
make run-chaining       # build and run the chaining tests
make valgrind-open      # run open-addressing tests under valgrind
make valgrind-chaining  # run chaining tests under valgrind
make clean              # remove build artifacts
```

## References

- Cormen, Leiserson, Rivest, Stein, *Introduction to Algorithms*,
  Chapter 11 (Hash Tables)
- Knuth, *The Art of Computer Programming*, Vol. 3, Section 6.4 (Hashing)
