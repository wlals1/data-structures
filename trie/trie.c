#include "trie.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#define ALPHABET_SIZE 26

/*
 * Each node owns ALPHABET_SIZE child pointers. The child's array index
 * encodes its letter (index 0 == 'a'), so the letter is never stored
 * explicitly. is_end marks whether a word terminates at this node.
 */
struct TrieNode {
  struct TrieNode *children[ALPHABET_SIZE];
  bool is_end;
};

/* ---- internal helpers (file-local) ---- */

/* Allocate a zeroed node: all children NULL, is_end false. */
static TrieNode *create_node(void) {
  TrieNode *n = calloc(1, sizeof(TrieNode));
  assert(n != NULL); /* study build: surface allocation failure loudly */
  return n;
}

/* True iff the node has no children at all. */
static bool has_no_children(const TrieNode *node) {
  for (int i = 0; i < ALPHABET_SIZE; i++)
    if (node->children[i] != NULL)
      return false;
  return true;
}

/* Enforce the word policy: non-NULL, non-empty, lowercase 'a'-'z' only. */
static bool is_valid_word(const char *word) {
  if (word == NULL || *word == '\0')
    return false;
  for (const char *c = word; *c; c++)
    if (*c < 'a' || *c > 'z')
      return false;
  return true;
}

/* ---- public API ---- */

TrieNode *trie_create(void) { return create_node(); }

void trie_insert(TrieNode *root, const char *word) {
  if (root == NULL || !is_valid_word(word))
    return;

  TrieNode *cur = root;
  for (const char *c = word; *c; c++) {
    int idx = *c - 'a';
    assert(idx >= 0 && idx < ALPHABET_SIZE); /* guaranteed by is_valid_word */
    if (cur->children[idx] == NULL)
      cur->children[idx] = create_node(); /* lazily create the path */
    cur = cur->children[idx];
  }
  cur->is_end = true; /* mark the terminal node as a word */
}

bool trie_search(const TrieNode *root, const char *word) {
  if (root == NULL || !is_valid_word(word))
    return false;

  const TrieNode *cur = root;
  for (const char *c = word; *c; c++) {
    int idx = *c - 'a';
    assert(idx >= 0 && idx < ALPHABET_SIZE);
    if (cur->children[idx] == NULL)
      return false; /* path breaks: word is not present */
    cur = cur->children[idx];
  }
  /* Reaching the node is not enough; it must be a word boundary. */
  return cur->is_end;
}

/*
 * Recursively delete `word` from the subtree rooted at `node`.
 * Returns the (possibly NULL) node that the parent should link back,
 * so a freed node automatically clears the parent's child slot.
 *
 * `node` is never NULL here: the only NULL entry point (the root) is
 * filtered by trie_delete, and each recursive call checks the child
 * slot before descending.
 */
static TrieNode *delete_helper(TrieNode *node, const char *word) {
  /* Base case: the whole word has been consumed; `node` is its terminal. */
  if (*word == '\0') {
    if (!node->is_end)
      return node; /* word was never inserted; leave the trie untouched */
    node->is_end = false;
    /* Free the terminal only if it ends no word and has no children. */
    if (has_no_children(node)) {
      free(node);
      return NULL;
    }
    return node;
  }

  /* Recursive case: descend one character. */
  int idx = *word - 'a';
  if (node->children[idx] == NULL)
    return node; /* path does not exist; word is absent */
  node->children[idx] = delete_helper(node->children[idx], word + 1);

  /* Unwinding bottom-up: prune this node if it is now useless, i.e. it
   * neither ends a word nor lies on any other word's path. */
  if (has_no_children(node) && !node->is_end) {
    free(node);
    return NULL;
  }
  return node;
}

void trie_delete(TrieNode *root, const char *word) {
  if (root == NULL || !is_valid_word(word))
    return;
  delete_helper(root, word);
}

/* Post-order free: release all children before the node itself, so no
 * child pointer is read after its parent is freed. NULL children are
 * absorbed by the base case, so no per-child NULL check is needed. */
void trie_destroy(TrieNode *root) {
  if (root == NULL)
    return;
  for (int i = 0; i < ALPHABET_SIZE; i++)
    trie_destroy(root->children[i]);
  free(root);
}
