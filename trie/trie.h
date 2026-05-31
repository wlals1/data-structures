#ifndef TRIE_H
#define TRIE_H

#include <stdbool.h>

/*
 * Trie (prefix tree) over the lowercase English alphabet ('a'-'z').
 *
 * The node type is opaque: callers hold a TrieNode* but cannot see the
 * struct layout, which keeps the internal representation private.
 *
 * Word policy: every word must be non-NULL, non-empty, and contain only
 * characters in the range 'a'-'z'. Words that violate this policy are
 * silently ignored by insert/delete and treated as "not found" by search.
 */
typedef struct TrieNode TrieNode;

/* Create an empty trie. Returns the root node (never NULL on success). */
TrieNode *trie_create(void);

/* Insert a word. No-op if root is NULL or the word is invalid. */
void trie_insert(TrieNode *root, const char *word);

/* Return true iff the exact word was inserted and not later deleted. */
bool trie_search(const TrieNode *root, const char *word);

/* Remove a word. Shared prefixes of other words are preserved.
 * No-op if root is NULL, the word is invalid, or the word is absent. */
void trie_delete(TrieNode *root, const char *word);

/* Free the entire trie, including the root. */
void trie_destroy(TrieNode *root);

#endif /* TRIE_H */
