/*
 * main.c
 *
 * Unit tests for 2-3 tree implementation.
 */

#include "two_three_tree.h"

#include <stdio.h>

/* === Test helpers === */

/* Forward-declared internal symbols used only for assertions in tests.
 * In a strict library boundary these would be hidden; exposed here so
 * tests can introspect the tree without depending on internals via API. */
static int  test_passed = 0;
static int  test_failed = 0;

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

static void test_leaf_delete_simple(void) {
    printf("Test 1: leaf delete (no underflow)\n");
    Node *root = NULL;
    tree_insert(&root, 10);
    tree_insert(&root, 20);

    tree_delete(&root, 10);
    CHECK(root != NULL, "root not null");
    CHECK(root->num_keys == 1, "num_keys == 1");
    CHECK(root->keys[0] == 20, "keys[0] == 20");

    tree_destroy(&root);
}

static void test_delete_missing_key(void) {
    printf("Test 2: delete non-existent key (no change)\n");
    Node *root = NULL;
    tree_insert(&root, 10);
    tree_insert(&root, 20);

    tree_delete(&root, 999);
    CHECK(root->num_keys == 2, "num_keys unchanged");
    CHECK(root->keys[0] == 10 && root->keys[1] == 20, "keys unchanged");

    tree_destroy(&root);
}

static void test_redistribute_right_sibling(void) {
    printf("Test 3: leaf redistribute (right sibling)\n");
    Node *root = NULL;
    int keys[] = { 20, 10, 30, 40 };
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        tree_insert(&root, keys[i]);
    }
    /* Tree: [20] / [10] [30,40] */

    tree_delete(&root, 10);
    /* Expected: [30] / [20] [40] */
    CHECK(root->keys[0] == 30, "root == 30");
    CHECK(root->children[0]->keys[0] == 20, "left == 20");
    CHECK(root->children[1]->keys[0] == 40, "right == 40");

    tree_destroy(&root);
}

static void test_redistribute_left_sibling(void) {
    printf("Test 4: leaf redistribute (left sibling)\n");
    Node *root = NULL;
    int keys[] = { 20, 10, 30, 5 };
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        tree_insert(&root, keys[i]);
    }
    /* Tree: [20] / [5,10] [30] */

    tree_delete(&root, 30);
    /* Expected: [10] / [5] [20] */
    CHECK(root->keys[0] == 10, "root == 10");
    CHECK(root->children[0]->keys[0] == 5, "left == 5");
    CHECK(root->children[1]->keys[0] == 20, "right == 20");

    tree_destroy(&root);
}

static void test_merge_right_sibling(void) {
    printf("Test 5: leaf merge (right sibling)\n");
    Node *root = NULL;
    int keys[] = { 20, 10, 30, 40, 35 };
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        tree_insert(&root, keys[i]);
    }
    /* Tree: [20, 35] / [10] [30] [40] */

    tree_delete(&root, 10);
    /* Expected: [35] / [20,30] [40] */
    CHECK(root->num_keys == 1, "root has 1 key");
    CHECK(root->keys[0] == 35, "root == 35");
    CHECK(root->children[0]->num_keys == 2, "left has 2 keys");
    CHECK(root->children[0]->keys[0] == 20, "left[0] == 20");
    CHECK(root->children[0]->keys[1] == 30, "left[1] == 30");
    CHECK(root->children[1]->keys[0] == 40, "right == 40");

    tree_destroy(&root);
}

static void test_merge_left_sibling(void) {
    printf("Test 6: leaf merge (left sibling)\n");
    Node *root = NULL;
    int keys[] = { 20, 10, 30, 40, 35 };
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        tree_insert(&root, keys[i]);
    }

    tree_delete(&root, 30);
    /* Expected: [35] / [10,20] [40] */
    CHECK(root->keys[0] == 35, "root == 35");
    CHECK(root->children[0]->keys[0] == 10, "left[0] == 10");
    CHECK(root->children[0]->keys[1] == 20, "left[1] == 20");
    CHECK(root->children[1]->keys[0] == 40, "right == 40");

    tree_destroy(&root);
}

static void test_internal_delete_successor(void) {
    printf("Test 7: internal-key delete via successor swap\n");
    Node *root = NULL;
    int keys[] = { 20, 10, 30, 50, 60, 40 };
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        tree_insert(&root, keys[i]);
    }
    /* Tree: [20, 50] / [10] [30,40] [60] */

    tree_delete(&root, 20);
    /* Expected: [30, 50] / [10] [40] [60] */
    CHECK(root->keys[0] == 30, "root[0] == 30");
    CHECK(root->keys[1] == 50, "root[1] == 50");
    CHECK(root->children[1]->keys[0] == 40, "mid == 40");

    tree_destroy(&root);
}

static void test_cascade_to_root(void) {
    printf("Test 8: cascade underflow reaches root (height -1)\n");
    Node *root = NULL;
    int keys[] = { 20, 10, 30 };
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
        tree_insert(&root, keys[i]);
    }
    /* Tree: [20] / [10] [30] (all 2-nodes) */

    tree_delete(&root, 10);
    /* Expected: tree shrinks to a single leaf [20, 30] */
    CHECK(root->num_keys == 2, "single node with 2 keys");
    CHECK(root->keys[0] == 20 && root->keys[1] == 30, "keys = [20, 30]");

    tree_destroy(&root);
}

static void test_full_sequence(void) {
    printf("Test 9: full insert/delete sequence (alphabet-mapped)\n");
    Node *root = NULL;

    /* Insert: D, A1, T1, A2, S, T2, R1, U1, C, T3, U2, R2, E
     * Mapped to integers in alphabetical/label order:
     *   A1=1, A2=2, C=3, D=4, E=5,
     *   R1=6, R2=7, S=8,
     *   T1=9, T2=10, T3=11,
     *   U1=12, U2=13
     */
    int insert_seq[] = { 4, 1, 9, 2, 8, 10, 6, 12, 3, 11, 13, 7, 5 };
    for (size_t i = 0; i < sizeof(insert_seq) / sizeof(insert_seq[0]); i++) {
        tree_insert(&root, insert_seq[i]);
    }

    /* Delete: A1, T1, T2, T3 */
    int delete_seq[] = { 1, 9, 10, 11 };
    for (size_t i = 0; i < sizeof(delete_seq) / sizeof(delete_seq[0]); i++) {
        tree_delete(&root, delete_seq[i]);
    }

    /* Expected final tree:
     *           [R2=7]
     *          /      \
     *       [D=4]    [U1=12]
     *       /  \      /   \
     *  [A2,C][E,R1] [S]  [U2]
     *  [2,3][5,6]  [8]   [13]
     */
    CHECK(root->keys[0] == 7, "root == R2(7)");
    CHECK(root->children[0]->keys[0] == 4, "left internal == D(4)");
    CHECK(root->children[1]->keys[0] == 12, "right internal == U1(12)");
    CHECK(root->children[0]->children[0]->keys[0] == 2, "[A2,C][0] == 2");
    CHECK(root->children[0]->children[0]->keys[1] == 3, "[A2,C][1] == 3");
    CHECK(root->children[0]->children[1]->keys[0] == 5, "[E,R1][0] == 5");
    CHECK(root->children[0]->children[1]->keys[1] == 6, "[E,R1][1] == 6");
    CHECK(root->children[1]->children[0]->keys[0] == 8, "[S] == 8");
    CHECK(root->children[1]->children[1]->keys[0] == 13, "[U2] == 13");

    tree_destroy(&root);
}

/* === Entry point === */

int main(void) {
    test_leaf_delete_simple();
    test_delete_missing_key();
    test_redistribute_right_sibling();
    test_redistribute_left_sibling();
    test_merge_right_sibling();
    test_merge_left_sibling();
    test_internal_delete_successor();
    test_cascade_to_root();
    test_full_sequence();

    printf("\n=== Results: %d passed, %d failed ===\n",
           test_passed, test_failed);
    return (test_failed == 0) ? 0 : 1;
}
