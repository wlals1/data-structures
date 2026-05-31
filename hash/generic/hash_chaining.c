#include <stdbool.h>
#include <stdlib.h>
#define M 13
#define R 7

typedef struct Slot {
  int key, value;
  struct Slot *next;
} Slot;

typedef struct HashTable {
  Slot *buckets[M];
} HashTable;

HashTable *ht_create(void) { return calloc(1, sizeof(HashTable)); }

static int h1(int k) { return k % M; }

bool ht_insert(HashTable *ht, int key, int value) {
  int idx = h1(key);
  Slot *s = ht->buckets[idx];
  for (; s; s = s->next) {
    if (s->key == key) {
      s->value = value;
      return true;
    }
  }
  Slot *n = calloc(1, sizeof(Slot));
  if (n == NULL)
    return false;
  n->key = key;
  n->value = value;
  n->next = ht->buckets[idx];
  ht->buckets[idx] = n;
  return true;
}

bool ht_delete(HashTable *ht, int key) {
  int idx = h1(key);
  Slot *prev = NULL, *del = ht->buckets[idx];
  while (del && del->key != key) {
    prev = del;
    del = del->next;
  }
  if (del == NULL)
    return false;
  if (prev == NULL)
    ht->buckets[idx] = del->next;
  else
    prev->next = del->next;
  free(del);
  return true;
}

bool ht_search(const HashTable *ht, int key, int *out_value) {
  int idx = h1(key);
  // buckets[idx] 리스트 순회, key 같으면 *out_value 채우고 true
  for (const Slot *s = ht->buckets[idx]; s; s = s->next) {
    if (s->key == key) {
      *out_value = s->value;
      return true;
    }
  }
  return false;
}

void ht_destroy(HashTable *ht) {
  if (ht == NULL)
    return;
  for (int i = 0; i < M; i++) {
    Slot *s = ht->buckets[i];
    while (s) {
      Slot *p = s;
      s = s->next;
      free(p);
    }
  }
  free(ht);
}
