# Trie

A prefix tree over the lowercase English alphabet, implemented in C,
supporting insert, search, delete, and destroy operations.

## Properties

- 26-ary tree: each node has up to 26 children, one per letter `'a'`–`'z'`.
- A child's array index encodes its letter (index `0` == `'a'`), so the
  letter itself is never stored.
- An `is_end` flag marks nodes where a word terminates. **Reaching a node
  is not the same as the word existing** — only `is_end == true` counts.
- Children are created lazily: a child pointer is `NULL` until some word
  actually requires that letter. A `NULL` child means "no word continues
  this way," which is what makes prefix lookups cheap.

## Operations

`L` = length of the word, `N` = total number of nodes in the trie.

| Operation | Time complexity | Space complexity   |
|-----------|-----------------|--------------------|
| Search    | O(L)            | O(1)               |
| Insert    | O(L)            | O(1)               |
| Delete    | O(L)            | O(L) stack         |
| Destroy   | O(N)            | O(H) stack         |

Notes:

- Search and insert are iterative, so they use no call stack — the cost
  depends only on the word length, never on how many words are stored.
- Delete and destroy are recursive; their stack depth is bounded by the
  longest word (`L`) / the tree height (`H`), which for a trie equals the
  longest stored word.

## Files

| File       | Contents                          |
|------------|-----------------------------------|
| `trie.h`   | Public API and opaque `TrieNode`  |
| `trie.c`   | Implementation                    |
| `main.c`   | Unit tests                        |
| `Makefile` | Build configuration               |

## Build and run

```bash
make            # build
make run        # build and run tests
make valgrind   # run tests under valgrind (requires valgrind)
make clean      # remove build artifacts
```

## API style

The node type is **opaque**: callers receive a `TrieNode*` from
`trie_create` and pass it back to the other functions, but cannot see the
struct layout. The root is created once and the same pointer is reused;
operations mutate the trie in place rather than returning a new root.

```c
TrieNode *root = trie_create();
trie_insert(root, "cat");
trie_insert(root, "car");
bool found = trie_search(root, "cat");  /* true  */
trie_delete(root, "cat");               /* "car" is preserved */
trie_destroy(root);
```

### Word policy

Every word must be **non-NULL, non-empty, and contain only `'a'`–`'z'`**.
Invalid words are silently ignored by `insert`/`delete` and treated as
"not found" by `search`. The policy is enforced in a single place
(`is_valid_word`), so widening it later (e.g. to uppercase) is a one-spot
change.

## Algorithm notes

### Insert

Walk the word one character at a time. At each step, if the child for that
letter is `NULL`, allocate it (lazy creation), then descend. After the last
character, set `is_end = true` on the terminal node.

### Search

Walk the word the same way. If any required child is `NULL`, the path
breaks and the word is absent. If the walk completes, the word exists
**iff** the terminal node has `is_end == true` — this distinguishes a
stored word from a mere prefix of a longer word.

### Delete

Delete is the subtle operation. It proceeds in two conceptual parts as the
recursion descends and then unwinds:

1. **Unmark.** At the terminal node, clear `is_end`. This alone makes the
   word "not found." If the word was never stored (`is_end` already
   `false`), the trie is left untouched.
2. **Prune (optional cleanup).** While unwinding, free each node that has
   become useless — meaning it **ends no word and has no children**. A
   node is *kept* if either condition fails:
   - it still ends another word (`is_end == true`), or
   - another word still branches through it (it has a child).

   Pruning is done bottom-up (post-order) so that a parent only inspects
   its child slot after the child has already freed itself and reset the
   slot to `NULL` via the returned pointer.

Examples that the tests exercise:

- Deleting `"app"` while `"apple"` exists only clears `is_end`; the shared
  `app…le` path is preserved.
- Deleting `"card"` while `"car"` exists frees only the `d` node; the `r`
  node is kept because it still ends `"car"`.
- Deleting `"cat"` with no other word on its path frees the chain of nodes
  back up to the last branch point.

### Destroy

A post-order traversal: free all children before freeing the node itself,
so no child pointer is dereferenced after its parent has been freed.
`NULL` children are handled by the function's own base case, so no
per-child `NULL` check is needed.

## References

- Sedgewick, *Algorithms*, Chapter 5.2 (Tries)
- Knuth, *The Art of Computer Programming*, Vol. 3, Section 6.3
  (Digital Searching)
