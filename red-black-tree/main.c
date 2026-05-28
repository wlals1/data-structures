/*
 * main.c
 *
 * Unit tests for the red-black tree implementation.
 *
 * Functional tests use a small CHECK macro and report a pass/fail
 * count. Randomized stress tests then perform thousands of inserts and
 * deletes, re-validating every red-black invariant after each operation.
 */

#include "rb_tree.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* === Test framework === */

static int test_passed = 0;
static int test_failed = 0;

#define CHECK(cond, msg)                                              \
    do {                                                              \
        if (cond) {                                                   \
            test_passed++;                                            \
        } else {                                                      \
            test_failed++;                                            \
            fprintf(stderr, "  FAIL: %s (%s:%d)\n",                   \
                    msg, __FILE__, __LINE__);                         \
        }                                                             \
    } while (0)

/* === Functional tests === */

static void test_insert_keeps_valid(void) {
    printf("Test 1: insert keeps tree valid\n");
    Node *root = NIL;
    int keys[] = { 7, 3, 18, 10, 22, 8, 11, 26 };
    int n = (int)(sizeof(keys) / sizeof(keys[0]));

    for (int i = 0; i < n; i++) {
        rb_insert(&root, keys[i]);
        CHECK(rb_is_valid(root), "valid after insert");
    }
    /* Standard CLRS shape: the root ends up as 7, colored black. */
    CHECK(rb_search(root, 7), "7 present");
    CHECK(rb_search(root, 26), "26 present");

    rb_destroy(&root);
}

static void test_search(void) {
    printf("Test 2: search present and absent keys\n");
    Node *root = NIL;
    int keys[] = { 50, 30, 70, 20, 40 };
    for (int i = 0; i < 5; i++) {
        rb_insert(&root, keys[i]);
    }

    CHECK(rb_search(root, 50), "find 50");
    CHECK(rb_search(root, 20), "find 20");
    CHECK(rb_search(root, 40), "find 40");
    CHECK(!rb_search(root, 999), "not find 999");
    CHECK(!rb_search(root, 0), "not find 0");

    rb_destroy(&root);
}

static void test_delete_leaf(void) {
    printf("Test 3: delete a leaf\n");
    Node *root = NIL;
    rb_insert(&root, 50);
    rb_insert(&root, 30);
    rb_insert(&root, 70);

    rb_delete(&root, 30);
    CHECK(rb_is_valid(root), "valid after delete");
    CHECK(!rb_search(root, 30), "30 removed");
    CHECK(rb_search(root, 50) && rb_search(root, 70), "others present");

    rb_destroy(&root);
}

static void test_delete_two_children(void) {
    printf("Test 4: delete a node with two children\n");
    Node *root = NIL;
    int keys[] = { 50, 30, 70, 20, 40, 60, 80 };
    for (int i = 0; i < 7; i++) {
        rb_insert(&root, keys[i]);
    }

    rb_delete(&root, 30); /* has two children: 20 and 40 */
    CHECK(rb_is_valid(root), "valid after delete");
    CHECK(!rb_search(root, 30), "30 removed");
    CHECK(rb_search(root, 20) && rb_search(root, 40), "children present");

    rb_destroy(&root);
}

static void test_delete_root_repeatedly(void) {
    printf("Test 5: delete every node down to empty\n");
    Node *root = NIL;
    int keys[] = { 7, 3, 18, 10, 22, 8, 11, 26 };
    int n = (int)(sizeof(keys) / sizeof(keys[0]));
    for (int i = 0; i < n; i++) {
        rb_insert(&root, keys[i]);
    }

    int del[] = { 18, 11, 3, 10, 22, 7, 8, 26 };
    for (int i = 0; i < n; i++) {
        rb_delete(&root, del[i]);
        CHECK(rb_is_valid(root), "valid after delete");
        CHECK(!rb_search(root, del[i]), "deleted key absent");
    }
    CHECK(root == NIL, "tree empty at end");

    rb_destroy(&root);
}

static void test_duplicate_insert(void) {
    printf("Test 6: duplicate insert is a no-op\n");
    Node *root = NIL;
    rb_insert(&root, 50);
    rb_insert(&root, 50);
    rb_insert(&root, 50);

    CHECK(rb_search(root, 50), "50 present");
    CHECK(root->left == NIL && root->right == NIL, "still a single node");
    CHECK(rb_is_valid(root), "valid");

    rb_destroy(&root);
}

static void test_delete_missing(void) {
    printf("Test 7: delete absent key is a no-op\n");
    Node *root = NIL;
    rb_insert(&root, 50);
    rb_insert(&root, 30);

    rb_delete(&root, 999);
    CHECK(rb_is_valid(root), "valid");
    CHECK(rb_search(root, 50) && rb_search(root, 30), "tree intact");

    rb_destroy(&root);
}

/* === Stress tests === */

static void test_stress_insert_then_delete(void) {
    printf("Test 8: stress - bulk insert then bulk delete\n");
    Node *root = NIL;
    const int N = 2000;
    const int RANGE = 5000;
    bool ok = true;

    for (int i = 0; i < N && ok; i++) {
        rb_insert(&root, rand() % RANGE);
        ok = rb_is_valid(root);
    }
    for (int i = 0; i < N && ok; i++) {
        rb_delete(&root, rand() % RANGE);
        ok = rb_is_valid(root);
    }
    CHECK(ok, "valid through 2x2000 operations");

    rb_destroy(&root);
}

static void test_stress_mixed(void) {
    printf("Test 9: stress - randomized mixed operations\n");
    Node *root = NIL;
    const int OPS = 5000;
    const int RANGE = 1000;
    bool ok = true;

    for (int i = 0; i < OPS && ok; i++) {
        int key = rand() % RANGE;
        if (rand() % 2) {
            rb_insert(&root, key);
        } else {
            rb_delete(&root, key);
        }
        ok = rb_is_valid(root);
    }
    CHECK(ok, "valid through 5000 mixed operations");

    rb_destroy(&root);
}

/* === Entry point === */

int main(void) {
    srand((unsigned)time(NULL));

    test_insert_keeps_valid();
    test_search();
    test_delete_leaf();
    test_delete_two_children();
    test_delete_root_repeatedly();
    test_duplicate_insert();
    test_delete_missing();
    test_stress_insert_then_delete();
    test_stress_mixed();

    printf("\n=== Results: %d passed, %d failed ===\n",
           test_passed, test_failed);
    return (test_failed == 0) ? 0 : 1;
}
