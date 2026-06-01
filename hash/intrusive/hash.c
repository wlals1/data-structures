#include "hash.h"

#include <stdlib.h>

#define M 13 /* bucket count; prime to spread keys across all buckets */

/* The table owns only the bucket array — pointers to chain heads.
 * The nodes themselves live inside the caller's data structures. */
struct HashTable {
  struct hnode *buckets[M];
};

/* Hash a key to a bucket index. */
static int h1(int key) { return key % M; }

HashTable *ht_create(void) { return calloc(1, sizeof(HashTable)); }

/* Head insertion: the caller's node is linked at the front of its bucket.
 * No allocation — the library only rewires pointers. */
void ht_insert(HashTable *ht, struct hnode *node, int key) {
  int idx = h1(key);
  node->key = key;
  node->next = ht->buckets[idx];
  ht->buckets[idx] = node;
}

/* Walk the bucket's chain and return the node whose key matches. */
hnode *ht_search(const HashTable *ht, int key) {
  int idx = h1(key);
  hnode *n = ht->buckets[idx];
  while (n && n->key != key)
    n = n->next;
  return n; /* the match, or NULL if the chain ran out */
}

/* Unlink the matching node from its chain. The node's memory is untouched. */
bool ht_delete(HashTable *ht, int key) {
  int idx = h1(key);
  hnode *prev = NULL, *curr = ht->buckets[idx];
  while (curr && curr->key != key) {
    prev = curr;
    curr = curr->next;
  }
  if (curr == NULL)
    return false; /* not present */
  if (prev == NULL)
    ht->buckets[idx] = curr->next; /* removing the head */
  else
    prev->next = curr->next; /* removing a middle/tail node */
  return true;
}

/* Free the table struct alone; nodes belong to the caller. */
void ht_destroy(HashTable *ht) { free(ht); }
