# Hash Table (intrusive)

A separate-chaining hash table where the list node is **embedded in the
user's data structure** rather than allocated by the library. Written in C.
This mirrors how the Linux kernel builds its data structures (`list_head`,
`hlist_node`, `rb_node`), and contrasts with the generic version in
`../generic`, where the library owns the nodes and copies keys/values into
them.

## The core idea

In the generic version the library defines a `Slot` that holds the key and
value, allocates one per entry, and copies the data in. Here it is inverted:
the user's own struct carries the link, and the library only ever touches
that link.

```c
struct hnode { struct hnode *next; int key; };   /* the embedded link */

struct process {        /* user data — owns the link by value */
    struct hnode link;  /* embedded, NOT a pointer */
    int value;
};
```

The link is embedded **by value**, not as a pointer. That is what makes
`container_of` work: because the `hnode` lives at a fixed offset inside the
`process`, the address of the link minus that offset is the address of the
`process`.

```c
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - offsetof(type, member)))
```

`offsetof(struct process, link)` is the distance (in bytes) from the start
of a `process` to its `link` member — a property of the type, fixed for
every instance. Subtracting it from a link's address recovers the enclosing
`process`. The library walks `hnode` chains; the user converts a found
`hnode` back into a `process` with `container_of`.

## Who owns what

| Thing | Owner | Notes |
|-------|-------|-------|
| `HashTable` + bucket array | library | created by `ht_create`, freed by `ht_destroy` |
| `hnode` (the link) | user | lives inside the user's struct |
| the data (`process`) | user | user allocates and frees it |

Because the user owns the data, the library never calls `malloc` or `free`
on nodes. `ht_insert` links a node the caller already built; `ht_delete`
only unlinks it; `ht_destroy` frees the table struct alone. Freeing the
data is the caller's job — and must happen *after* unlinking, never before
(freeing a `process` also destroys its embedded link, so reading
`node->next` afterwards is use-after-free).

## API

```c
HashTable *ht_create(void);
void ht_insert(HashTable *ht, struct hnode *node, int key);
struct hnode *ht_search(const HashTable *ht, int key);  /* NULL if absent */
bool ht_delete(HashTable *ht, int key);                 /* unlink only */
void ht_destroy(HashTable *ht);                          /* frees table only */
```

`ht_search` returns the `hnode`, not a value — the library does not know the
data's type, so it cannot extract a value. The caller recovers the data:

```c
struct hnode *h = ht_search(ht, 18);
if (h) {
    process *p = container_of(h, process, link);
    use(p->value);
}
```

The key is stored in the `hnode` so the library can compare without knowing
the data type. (A fully type-agnostic design would instead take a
user-supplied comparison callback — the kernel's approach — and keep the key
out of the node entirely. That is left as a later extension.)

## Generic vs intrusive

| | generic | intrusive |
|---|---|---|
| Node | `Slot`, allocated by library | embedded in user data |
| Key/value | copied into the node | stay in the user's struct |
| Library knows the data type? | no (copies key) | no (key in node, data via caller) |
| Node allocation | library (`malloc`) | user |
| Node free | library | user |
| Same item in multiple structures | needs a node per structure | one embedded link per structure |
| Coupling | low | the link couples data to the structure |

Intrusive avoids per-entry allocation and lets one object belong to several
structures at once (one embedded link each), at the cost of pushing memory
and lifetime management onto the caller. The kernel chooses it for exactly
these reasons — performance and multi-membership matter, and the kernel owns
all the types it stores.

## Complexity

`n` = entries, `M` = bucket count, load factor α = n/M.

| Operation | Average | Worst case |
|-----------|---------|------------|
| Insert    | O(1)    | O(1)       |
| Search    | O(1)    | O(n)       |
| Delete    | O(1)    | O(n)       |

Insert is unconditionally O(1) (head insertion, no allocation). Search and
delete walk one bucket's chain, so they degrade as α rises and chains grow.

## Files

| File       | Contents                                       |
|------------|------------------------------------------------|
| `hash.h`   | `hnode`, `container_of`, opaque `HashTable`, API |
| `hash.c`   | Chain linking logic (no knowledge of the data) |
| `main.c`   | Defines `process`, embeds the link, tests      |
| `Makefile` | Build configuration                            |

## Build and run

```bash
make            # build
make run        # build and run tests
make valgrind   # run under valgrind
make clean      # remove build artifacts
```

## References

- Linux kernel source: `include/linux/list.h`, `include/linux/hashtable.h`
  (`container_of`, `hlist_node`, `list_for_each_entry`)
- Cormen et al., *Introduction to Algorithms*, Chapter 11 (Hash Tables)
