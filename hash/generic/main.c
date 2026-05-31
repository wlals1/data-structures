#include "hash.h"
#include <stdio.h>

static void check(const char *label, bool got, bool expected) {
  printf("%-36s got=%-5s expected=%-5s %s\n", label, got ? "true" : "false",
         expected ? "true" : "false", got == expected ? "OK" : "<<< FAIL");
}

static void check_val(const char *label, int got, int expected) {
  printf("%-36s got=%-5d expected=%-5d %s\n", label, got, expected,
         got == expected ? "OK" : "<<< FAIL");
}

int main(void) {
  HashTable *ht = ht_create();
  int v;

  // --- 1) 기본 insert → search ---
  ht_insert(ht, 5, 100);
  ht_insert(ht, 18, 200); // 18 % 13 = 5  → 5와 충돌 (같은 버킷)
  check("search 5 found", ht_search(ht, 5, &v), /* expected? */ true);
  check_val("  value of 5", v, /* expected? */ 100);
  check("search 18 found", ht_search(ht, 18, &v), /* expected? */ true);
  check_val("  value of 18", v, /* expected? */ 200);
  check("search 99 (absent)", ht_search(ht, 99, &v), /* expected? */ false);

  // --- 2) 중복 키 insert → 값 갱신 ---
  ht_insert(ht, 5, 999);
  ht_search(ht, 5, &v);
  check_val("update 5 -> 999", v, /* expected? */ 999);

  // --- 3) delete 후 search ---
  ht_delete(ht, 18);
  check("after del 18: search 18", ht_search(ht, 18, &v),
        /* expected? */ false);
  check("after del 18: search 5", ht_search(ht, 5, &v), /* expected? */ true);

  // --- 4) 버그② 검증: DELETED 슬롯 재사용 + tombstone 통과 ---
  // 새 깨끗한 테이블로 (앞 테스트 상태와 섞이지 않게)
  HashTable *h2t = ht_create();
  ht_insert(h2t, 5, 50);   // 슬롯 5
  ht_insert(h2t, 18, 180); // 슬롯 8 (5와 충돌 후 탐사)
  ht_insert(h2t, 31, 310); // 슬롯 9

  // tombstone 통과 검증: 5 삭제해도 18, 31 검색되어야
  ht_delete(h2t, 5);
  check("del 5: search 18 (tombstone pass)", ht_search(h2t, 18, &v), true);
  check_val("  value 18", v, 180);
  check("del 5: search 31 (tombstone pass)", ht_search(h2t, 31, &v), true);

  // DELETED 재사용 검증: 44 삽입 → 슬롯 5(옛 5자리) 재사용
  ht_insert(h2t, 44, 440); // first_deleted=5 → 슬롯 5에 앉아야
  check("reuse: search 44", ht_search(h2t, 44, &v), true);
  check_val("  value 44", v, 440);
  // 5는 지웠으니 없어야
  check("5 still gone", ht_search(h2t, 5, &v), false);

  ht_destroy(h2t);

  // --- 5) delete한 키 재삽입 ---
  ht_insert(ht, 18, 777);
  check("reinsert 18", ht_search(ht, 18, &v), /* expected? */ true);
  check_val("  value of 18", v, /* expected? */ 777);

  ht_destroy(ht);
  printf("\ndone\n");
  return 0;
}