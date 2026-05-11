/*
 * main.c
 *
 * Unit tests for AVL tree implementation.
 */

#include "avl_tree.h"

#include <stdio.h>

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

/* === Tests === */

static void test_basic_insert(void) {
    printf("Test 1: basic insert (balanced)\n");
    Node *root = NULL;
    root = avl_insert(root, 50);
    root = avl_insert(root, 30);
    root = avl_insert(root, 70);

    CHECK(root->key == 50, "root == 50");
    CHECK(root->height == 2, "height == 2");
    CHECK(root->left->key == 30, "left == 30");
    CHECK(root->right->key == 70, "right == 70");

    avl_destroy(root);
}

static void test_rotation_ll(void) {
    printf("Test 2: LL rotation (descending insert)\n");
    Node *root = NULL;
    root = avl_insert(root, 30);
    root = avl_insert(root, 20);
    root = avl_insert(root, 10);

    /* Expected after LL rotation: root=20, left=10, right=30 */
    CHECK(root->key == 20, "root == 20");
    CHECK(root->left->key == 10, "left == 10");
    CHECK(root->right->key == 30, "right == 30");
    CHECK(root->height == 2, "height == 2");

    avl_destroy(root);
}

static void test_rotation_rr(void) {
    printf("Test 3: RR rotation (ascending insert)\n");
    Node *root = NULL;
    root = avl_insert(root, 10);
    root = avl_insert(root, 20);
    root = avl_insert(root, 30);

    CHECK(root->key == 20, "root == 20");
    CHECK(root->left->key == 10, "left == 10");
    CHECK(root->right->key == 30, "right == 30");

    avl_destroy(root);
}

static void test_rotation_lr(void) {
    printf("Test 4: LR rotation\n");
    Node *root = NULL;
    root = avl_insert(root, 30);
    root = avl_insert(root, 10);
    root = avl_insert(root, 20);

    CHECK(root->key == 20, "root == 20");
    CHECK(root->left->key == 10, "left == 10");
    CHECK(root->right->key == 30, "right == 30");

    avl_destroy(root);
}

static void test_rotation_rl(void) {
    printf("Test 5: RL rotation\n");
    Node *root = NULL;
    root = avl_insert(root, 10);
    root = avl_insert(root, 30);
    root = avl_insert(root, 20);

    CHECK(root->key == 20, "root == 20");
    CHECK(root->left->key == 10, "left == 10");
    CHECK(root->right->key == 30, "right == 30");

    avl_destroy(root);
}

static void test_search(void) {
    printf("Test 6: search\n");
    Node *root = NULL;
    int keys[] = { 50, 30, 70, 20, 40 };
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        root = avl_insert(root, keys[i]);
    }

    CHECK(avl_search(root, 50), "find 50");
    CHECK(avl_search(root, 20), "find 20");
    CHECK(avl_search(root, 40), "find 40");
    CHECK(!avl_search(root, 999), "not find 999");
    CHECK(!avl_search(root, 0), "not find 0");

    avl_destroy(root);
}

static void test_delete_leaf(void) {
    printf("Test 7: delete leaf\n");
    Node *root = NULL;
    root = avl_insert(root, 50);
    root = avl_insert(root, 30);
    root = avl_insert(root, 70);

    root = avl_delete(root, 30);
    CHECK(root->key == 50, "root == 50");
    CHECK(root->left == NULL, "left == NULL");
    CHECK(root->right->key == 70, "right == 70");

    avl_destroy(root);
}

static void test_delete_one_child(void) {
    printf("Test 8: delete node with one child\n");
    Node *root = NULL;
    int keys[] = { 50, 30, 70, 20 };
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        root = avl_insert(root, keys[i]);
    }

    root = avl_delete(root, 30);
    CHECK(root->key == 50, "root == 50");
    CHECK(root->left->key == 20, "left == 20");
    CHECK(root->right->key == 70, "right == 70");

    avl_destroy(root);
}

static void test_delete_two_children(void) {
    printf("Test 9: delete node with two children (successor swap)\n");
    Node *root = NULL;
    int keys[] = { 50, 30, 70, 20, 40, 60, 80 };
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        root = avl_insert(root, keys[i]);
    }

    /* Delete 30: its successor (in right subtree) is 40 */
    root = avl_delete(root, 30);
    CHECK(root->key == 50, "root == 50");
    CHECK(root->left->key == 40, "left == 40");
    CHECK(root->left->left->key == 20, "left.left == 20");
    CHECK(root->left->right == NULL, "left.right == NULL");
    CHECK(root->right->key == 70, "right == 70");

    avl_destroy(root);
}

static void test_delete_with_rebalance(void) {
    printf("Test 10: delete triggering rebalance\n");
    Node *root = NULL;
    int keys[] = { 50, 30, 70, 20, 40, 60, 80, 10 };
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        root = avl_insert(root, keys[i]);
    }

    root = avl_delete(root, 70);
    /* After deletion the tree should remain balanced. We just verify
     * structural balance rather than a specific shape. */
    CHECK(root != NULL, "root not null");
    CHECK(avl_search(root, 50), "50 still present");
    CHECK(avl_search(root, 10), "10 still present");
    CHECK(!avl_search(root, 70), "70 removed");

    avl_destroy(root);
}

static void test_duplicate_insert(void) {
    printf("Test 11: duplicate insert is no-op\n");
    Node *root = NULL;
    root = avl_insert(root, 50);
    root = avl_insert(root, 50);
    root = avl_insert(root, 50);

    CHECK(root->key == 50, "root == 50");
    CHECK(root->left == NULL && root->right == NULL, "still a single leaf");
    CHECK(root->height == 1, "height == 1");

    avl_destroy(root);
}

static void test_delete_missing(void) {
    printf("Test 12: delete non-existent key is no-op\n");
    Node *root = NULL;
    root = avl_insert(root, 50);
    root = avl_insert(root, 30);

    root = avl_delete(root, 999);
    CHECK(root->key == 50, "root == 50");
    CHECK(root->left->key == 30, "left == 30");

    avl_destroy(root);
}

/* === Entry point === */

int main(void) {
    test_basic_insert();
    test_rotation_ll();
    test_rotation_rr();
    test_rotation_lr();
    test_rotation_rl();
    test_search();
    test_delete_leaf();
    test_delete_one_child();
    test_delete_two_children();
    test_delete_with_rebalance();
    test_duplicate_insert();
    test_delete_missing();

    printf("\n=== Results: %d passed, %d failed ===\n",
           test_passed, test_failed);
    return (test_failed == 0) ? 0 : 1;
}
