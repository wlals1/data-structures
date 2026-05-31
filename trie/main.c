#include "trie.h"

#include <stdio.h>

/* Print a labeled comparison of actual vs. expected and flag mismatches. */
static void check(const char *label, bool got, bool expected) {
  printf("%-28s got=%-5s expected=%-5s %s\n", label, got ? "true" : "false",
         expected ? "true" : "false", got == expected ? "OK" : "<<< FAIL");
}

int main(void) {
  TrieNode *root = trie_create();

  /* --- insert --- */
  trie_insert(root, "cat");
  trie_insert(root, "car");
  trie_insert(root, "card");
  trie_insert(root, "app");
  trie_insert(root, "apple");

  /* --- basic search --- */
  check("search cat", trie_search(root, "cat"), true);
  check("search car", trie_search(root, "car"), true);
  check("search card", trie_search(root, "card"), true);
  check("search ca", trie_search(root, "ca"), false); /* prefix, not a word */
  check("search cards", trie_search(root, "cards"), false); /* path breaks */
  check("search app", trie_search(root, "app"), true);
  check("search apple", trie_search(root, "apple"), true);

  /* --- delete: removing "app" must keep "apple" alive --- */
  trie_delete(root, "app");
  check("after del app: app", trie_search(root, "app"), false);
  check("after del app: apple", trie_search(root, "apple"), true);

  /* --- delete: removing "card" must keep "car" alive (branch point) --- */
  trie_delete(root, "card");
  check("after del card: card", trie_search(root, "card"), false);
  check("after del card: car", trie_search(root, "car"), true);

  /* --- delete: "cat" has no other word on its path, so nodes are freed --- */
  trie_delete(root, "cat");
  check("after del cat: cat", trie_search(root, "cat"), false);

  /* --- edge cases: deleting absent/invalid input must not crash --- */
  trie_delete(root, "zzz"); /* absent word */
  trie_delete(root, "");    /* empty string (ignored by policy) */
  trie_delete(root, "ABC"); /* uppercase (ignored by policy) */
  trie_delete(NULL, "cat"); /* NULL trie */

  /* --- edge cases: invalid input search --- */
  check("search empty", trie_search(root, ""), false);
  check("search ABC", trie_search(root, "ABC"), false);

  trie_destroy(root);
  printf("\ndone\n");
  return 0;
}
