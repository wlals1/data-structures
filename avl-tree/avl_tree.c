/*
 * avl_tree.c
 *
 * AVL Tree implementation in C.
 * See avl_tree.h for usage and invariants.
 */

#include "avl_tree.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/* === Internal helpers === */

static int   max_int(int a, int b);
static Node *create_node(int key);
static int   get_height(const Node *node);
static int   get_balance(const Node *node);
static void  update_height(Node *node);
static Node *rotate_left(Node *node);
static Node *rotate_right(Node *node);
static Node *rebalance(Node *node);
static void  print_recursive(const Node *node, int depth);

/* === Helper definitions === */

static int max_int(int a, int b) {
    return (a >= b) ? a : b;
}

/**
 * Allocate a new leaf node with the given key.
 * Aborts on allocation failure.
 */
static Node *create_node(int key) {
    Node *node = malloc(sizeof(*node));
    assert(node != NULL);
    node->key = key;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
    return node;
}

/**
 * Return the height of a node. NULL is treated as height 0
 * so that leaves have height 1.
 */
static int get_height(const Node *node) {
    return (node == NULL) ? 0 : node->height;
}

/**
 * Return the balance factor: left subtree height - right subtree height.
 * NULL has balance factor 0.
 */
static int get_balance(const Node *node) {
    if (node == NULL) return 0;
    return get_height(node->left) - get_height(node->right);
}

/**
 * Recompute and store the height of a node based on its children.
 */
static void update_height(Node *node) {
    assert(node != NULL);
    node->height = 1 + max_int(get_height(node->left),
                                get_height(node->right));
}

/**
 * Perform a right rotation around `node` and return the new subtree root.
 *
 *       node              new_root
 *       /                 /     \
 *   new_root     →      x        node
 *   /     \            ...      /
 *  x       y                   y
 */
static Node *rotate_right(Node *node) {
    assert(node != NULL);
    assert(node->left != NULL);

    Node *new_root = node->left;
    node->left = new_root->right;
    update_height(node);
    new_root->right = node;
    update_height(new_root);
    return new_root;
}

/**
 * Perform a left rotation around `node` and return the new subtree root.
 *
 *   node                  new_root
 *      \                  /     \
 *   new_root      →    node       y
 *   /     \              \       ...
 *  x       y              x
 */
static Node *rotate_left(Node *node) {
    assert(node != NULL);
    assert(node->right != NULL);

    Node *new_root = node->right;
    node->right = new_root->left;
    update_height(node);
    new_root->left = node;
    update_height(new_root);
    return new_root;
}

/**
 * Rebalance a node after insert or delete.
 * Updates the height and applies one of LL, LR, RR, RL rotation if needed.
 * Returns the new subtree root.
 */
static Node *rebalance(Node *node) {
    update_height(node);
    int bf = get_balance(node);

    /* Left-heavy */
    if (bf > 1) {
        if (get_balance(node->left) >= 0) {
            /* LL */
            return rotate_right(node);
        }
        /* LR */
        node->left = rotate_left(node->left);
        return rotate_right(node);
    }

    /* Right-heavy */
    if (bf < -1) {
        if (get_balance(node->right) <= 0) {
            /* RR */
            return rotate_left(node);
        }
        /* RL */
        node->right = rotate_right(node->right);
        return rotate_left(node);
    }

    /* Balanced */
    return node;
}

/* === Public API === */

bool avl_search(const Node *root, int key) {
    if (root == NULL) return false;
    if (root->key == key) return true;
    if (key < root->key) return avl_search(root->left, key);
    return avl_search(root->right, key);
}

Node *avl_insert(Node *root, int key) {
    if (root == NULL) {
        return create_node(key);
    }

    if (key == root->key) {
        /* Duplicate: no-op */
        return root;
    }

    if (key < root->key) {
        root->left = avl_insert(root->left, key);
    } else {
        root->right = avl_insert(root->right, key);
    }

    return rebalance(root);
}

Node *avl_delete(Node *root, int key) {
    if (root == NULL) return NULL;

    if (key < root->key) {
        root->left = avl_delete(root->left, key);
    } else if (key > root->key) {
        root->right = avl_delete(root->right, key);
    } else {
        /* Found the node to delete */
        if (root->left == NULL && root->right == NULL) {
            free(root);
            return NULL;
        }
        if (root->right == NULL) {
            Node *child = root->left;
            free(root);
            return rebalance(child);
        }
        if (root->left == NULL) {
            Node *child = root->right;
            free(root);
            return rebalance(child);
        }

        /* Two children: copy inorder successor's key, delete successor */
        Node *successor = root->right;
        while (successor->left != NULL) {
            successor = successor->left;
        }
        root->key = successor->key;
        root->right = avl_delete(root->right, successor->key);
    }

    return rebalance(root);
}

void avl_destroy(Node *root) {
    if (root == NULL) return;
    avl_destroy(root->left);
    avl_destroy(root->right);
    free(root);
}

/* === Print (for debugging) === */

static void print_recursive(const Node *node, int depth) {
    if (node == NULL) return;
    print_recursive(node->right, depth + 1);
    for (int i = 0; i < depth; i++) printf("    ");
    printf("[%d] (h=%d)\n", node->key, node->height);
    print_recursive(node->left, depth + 1);
}

void avl_print(const Node *root) {
    if (root == NULL) {
        printf("(empty tree)\n");
        return;
    }
    printf("--- AVL tree ---\n");
    print_recursive(root, 0);
    printf("----------------\n");
}
