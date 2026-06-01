#include "hash.h"

#include <stdio.h>

/*
 * User data that embeds the hash link by value. Because `link` lives at a
 * fixed offset inside `process`, container_of can recover a `process*` from
 * a pointer to its `link`.
 */
typedef struct process {
  hnode link;
  int value;
} process;

/* Look up `key`, recover the process via container_of, compare its value. */
static void check(const char *label, HashTable *ht, int key, int expected) {
  struct hnode *h = ht_search(ht, key);
  if (!h) {
    printf("%-24s NOT FOUND (expected %d) <<< FAIL\n", label, expected);
    return;
  }
  process *p = container_of(h, process, link);
  printf("%-24s got=%-4d expected=%-4d %s\n", label, p->value, expected,
         p->value == expected ? "OK" : "<<< FAIL");
}

static void check_absent(const char *label, HashTable *ht, int key) {
  struct hnode *h = ht_search(ht, key);
  printf("%-24s %s\n", label, h == NULL ? "absent OK" : "<<< FAIL (found)");
}

int main(void) {
  HashTable *ht = ht_create();

  /* Stack-allocated data: no malloc/free needed, and the links stay valid
   * for the whole scope. Keys 5, 18, 31 all hash to bucket 5 (mod 13),
   * so this also exercises a collision chain. */
  process p[3] = {0};
  p[0].value = 100;
  ht_insert(ht, &p[0].link, 5);
  p[1].value = 200;
  ht_insert(ht, &p[1].link, 18);
  p[2].value = 300;
  ht_insert(ht, &p[2].link, 31);

  check("search 5", ht, 5, 100);
  check("search 18", ht, 18, 200);
  check("search 31", ht, 31, 300);
  check_absent("search 99", ht, 99);

  /* Delete the middle of the collision chain; neighbors must survive. */
  ht_delete(ht, 18);
  check_absent("after del 18", ht, 18);
  check("5 still there", ht, 5, 100);
  check("31 still there", ht, 31, 300);

  ht_destroy(ht); /* table only; p[] is on the stack */
  return 0;
}
