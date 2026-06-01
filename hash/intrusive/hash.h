#ifndef HASH_H
#define HASH_H

#include <stdbool.h>
#include <stddef.h> /* offsetof */

/*
 * Intrusive hash table over integer keys, separate chaining.
 *
 * The list node (hnode) is embedded by value inside the user's own data
 * structure; the library never allocates or frees nodes. The key is kept
 * in the node so the library can compare without knowing the data type.
 * The table itself (HashTable) is opaque and owned by the library.
 */

/* The embedded link. The user places this inside their own struct. */
typedef struct hnode {
  struct hnode *next;
  int key;
} hnode;

/*
 * Recover the enclosing struct from a pointer to its embedded member.
 * `ptr` points at a member of type-and-name `member` inside `type`;
 * subtracting the member's offset yields the address of `type`.
 */
#define container_of(ptr, type, member)                                        \
  ((type *)((char *)(ptr) - offsetof(type, member)))

typedef struct HashTable HashTable;

/* Create an empty table. Returns NULL on allocation failure. */
HashTable *ht_create(void);

/* Link an already-built node under `key`. The caller owns the node's memory. */
void ht_insert(HashTable *ht, struct hnode *node, int key);

/* Return the node matching `key`, or NULL if absent.
 * The caller recovers its data with container_of(result, type, member). */
hnode *ht_search(const HashTable *ht, int key);

/* Unlink the node matching `key`. Does NOT free it — that is the caller's
 * job, and must happen only after unlinking. Returns false if absent. */
bool ht_delete(HashTable *ht, int key);

/* Free the table struct only. Embedded nodes (and their data) are the
 * caller's responsibility. */
void ht_destroy(HashTable *ht);

#endif /* HASH_H */
