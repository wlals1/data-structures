#include "hash.h"
#include <stdlib.h>
#define M 13
#define R 7
#define EMPTY 0
#define OCCUPIED 1
#define DELETED 2

typedef struct {
  int key, value, state;
} Slot;

typedef struct HashTable {
  Slot table[M];
} HashTable;

HashTable *ht_create(void) { return calloc(1, sizeof(HashTable)); }

static int h1(int k) { return k % M; }
static int h2(int k) { return R - (k % R); }

static void slot_put(Slot *s, int key, int value) {
  s->key = key;
  s->value = value;
  s->state = OCCUPIED;
}

bool ht_insert(HashTable *ht, int key, int value) {
  int first_deleted = -1;
  for (int i = 0; i < M; i++) {
    int idx = (h1(key) + i * h2(key)) % M;
    Slot *s = &ht->table[idx];
    if (s->state == OCCUPIED) {
      if (s->key == key) {
        s->value = value;
        return true;
      }
    } else if (s->state == DELETED) {
      if (first_deleted == -1) {
        first_deleted = idx;
      }
    } else if (s->state == EMPTY) {
      int target = (first_deleted != -1) ? first_deleted : idx;
      slot_put(&ht->table[target], key, value);
      return true;
    }
  }

  if (first_deleted != -1) {
    slot_put(&ht->table[first_deleted], key, value);
    return true;
  }
  return false;
}

bool ht_search(const HashTable *ht, int key, int *out_value) {
  for (int i = 0; i < M; i++) {
    int idx = (h1(key) + i * h2(key)) % M;
    const Slot *s = &ht->table[idx];
    if (s->state == OCCUPIED && s->key == key) {
      *out_value = s->value;
      return true;
    } else if (s->state == EMPTY) {
      return false;
    }
    // DELETED / 다른 키 → 계속
  }
  return false;
}

bool ht_delete(HashTable *ht, int key) {
  for (int i = 0; i < M; i++) {
    int idx = (h1(key) + i * h2(key)) % M;
    Slot *s = &ht->table[idx]; // const 아님 — 수정하니까
    // OCCUPIED + 같은 키 → s->state = DELETED; return true;
    if (s->state == OCCUPIED && s->key == key) {
      s->state = DELETED;
      return true;
    } else if (s->state == EMPTY) {
      // EMPTY → return false;
      return false;
    }
    // DELETED / 다른 키 → 계속
  }
  return false;
}

void ht_destroy(HashTable *ht) { free(ht); }