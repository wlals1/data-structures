#ifndef HASH_H
#define HASH_H
#include <stdbool.h>

typedef struct HashTable HashTable;

HashTable *ht_create(void);
bool ht_insert(HashTable *ht, int key, int value);
/* Returns true and writes the value to *out_value if found.
 * If false, *out_value is left untouched — read it only when true. */
bool ht_search(const HashTable *ht, int key, int *out_value);
bool ht_delete(HashTable *ht, int key);
void ht_destroy(HashTable *ht);

#endif