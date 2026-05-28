/*
 * rb_tree.c
 *
 * Red-Black Tree implementation in C.
 * See rb_tree.h for usage and invariants.
 *
 * Based on CLRS, Introduction to Algorithms, Chapter 13.
 * Uses a single shared sentinel node (NIL) for all empty positions,
 * which lets rotation and fixup code read NIL->color and NIL->parent
 * without special-casing NULL.
 */

#include "rb_tree.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

/* === Sentinel === */

/*
 * The one and only NIL node. It is black, and rotation/delete code may
 * temporarily set NIL->parent to the relevant context node; that is
 * intentional and safe because there is exactly one NIL.
 */
static Node nil_sentinel = {
    .key    = 0,
    .color  = BLACK,
    .left   = NULL,
    .right  = NULL,
    .parent = NULL,
};
Node *NIL = &nil_sentinel;

/* === Internal helpers === */

static Node *create_node(int key);
static void  replace_child(Node **root, Node *old_child, Node *new_child);
static void  rotate_left(Node **root, Node *x);
static void  rotate_right(Node **root, Node *x);
static Node *bst_insert(Node **root, int key);
static void  insert_fixup(Node **root, Node *z);
static void  transplant(Node **root, Node *u, Node *v);
static Node *tree_minimum(Node *x);
static void  delete_fixup(Node **root, Node *x);
static Node *find_node(Node *root, int key);
static void  free_subtree(Node *node);
static void  print_recursive(const Node *node, int depth);

/* === Helper definitions === */

/**
 * Allocate a new red node with the given key and NIL children/parent.
 * New nodes are red so that insertion can only violate invariant 4
 * (no red-red), which is fixable locally, rather than invariant 5
 * (equal black-height), which is not. Aborts on allocation failure.
 */
static Node *create_node(int key) {
    Node *node = malloc(sizeof(*node));
    assert(node != NULL);
    node->key    = key;
    node->color  = RED;
    node->left   = NIL;
    node->right  = NIL;
    node->parent = NIL;
    return node;
}

/**
 * Repoint old_child's parent to new_child. Updates the parent's child
 * link (or *root if old_child was the root) but does NOT touch
 * new_child->parent. This is the three-way branch shared by both
 * rotations and by transplant; the Linux kernel calls its equivalent
 * __rb_change_child().
 */
static void replace_child(Node **root, Node *old_child, Node *new_child) {
    Node *parent = old_child->parent;
    if (parent == NIL) {
        *root = new_child;
    } else if (parent->left == old_child) {
        parent->left = new_child;
    } else {
        parent->right = new_child;
    }
}

/**
 * Left rotation around x. Its right child y moves up into x's place and
 * x becomes y's left child.
 *
 *     x                 y
 *    / \               / \
 *   a   y     ->      x   c
 *      / \           / \
 *     b   c         a   b
 *
 * Precondition: x->right != NIL. Pure structural operation: colors are
 * left untouched (the caller adjusts them).
 */
static void rotate_left(Node **root, Node *x) {
    assert(x != NIL);
    assert(x->right != NIL);

    Node *y = x->right;

    /* b becomes x's right subtree */
    x->right = y->left;
    if (y->left != NIL) {
        y->left->parent = x;
    }

    /* y takes x's place */
    y->parent = x->parent;
    replace_child(root, x, y);

    /* x becomes y's left child */
    y->left = x;
    x->parent = y;
}

/**
 * Right rotation around x: the mirror image of rotate_left.
 *
 *       x             y
 *      / \           / \
 *     y   c   ->    a   x
 *    / \               / \
 *   a   b             b   c
 *
 * Precondition: x->left != NIL.
 */
static void rotate_right(Node **root, Node *x) {
    assert(x != NIL);
    assert(x->left != NIL);

    Node *y = x->left;

    x->left = y->right;
    if (y->right != NIL) {
        y->right->parent = x;
    }

    y->parent = x->parent;
    replace_child(root, x, y);

    y->right = x;
    x->parent = y;
}

/**
 * Ordinary BST insertion (no color fixup). Returns the newly created
 * node, or NIL if the key already exists (duplicates are ignored).
 */
static Node *bst_insert(Node **root, int key) {
    Node *parent = NIL;
    Node *cur    = *root;

    while (cur != NIL) {
        parent = cur;
        if (key < cur->key) {
            cur = cur->left;
        } else if (key > cur->key) {
            cur = cur->right;
        } else {
            return NIL; /* duplicate */
        }
    }

    Node *z = create_node(key);
    z->parent = parent;
    if (parent == NIL) {
        *root = z;
    } else if (key < parent->key) {
        parent->left = z;
    } else {
        parent->right = z;
    }
    return z;
}

/**
 * Restore the red-black invariants after inserting red node z.
 *
 * The only possible violation is red-red between z and its parent. Let
 * p = parent, g = grandparent, u = uncle. The fix depends on u's color:
 *
 *   Case 1 (uncle red): recolor p and u black, g red, then continue the
 *     check two levels up at g. Pure recoloring, may propagate to the
 *     root; this is why recoloring can happen O(log n) times.
 *   Case 2 (uncle black, z is a "bent" child): rotate at p to turn the
 *     shape into Case 3.
 *   Case 3 (uncle black, z is a "straight" child): recolor and rotate at
 *     g; this terminates the loop.
 *
 * A rotation only ever happens in Case 2/3, which end the loop, so an
 * insert performs at most two rotations.
 */
static void insert_fixup(Node **root, Node *z) {
    while (z->parent->color == RED) {
        Node *g = z->parent->parent;

        if (z->parent == g->left) {
            Node *u = g->right;
            if (u->color == RED) {
                /* Case 1 */
                z->parent->color = BLACK;
                u->color = BLACK;
                g->color = RED;
                z = g;
            } else {
                if (z == z->parent->right) {
                    /* Case 2: bend left, then fall through to Case 3 */
                    z = z->parent;
                    rotate_left(root, z);
                }
                /* Case 3 */
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rotate_right(root, z->parent->parent);
            }
        } else {
            /* Mirror image of the above (parent is a right child) */
            Node *u = g->left;
            if (u->color == RED) {
                /* Case 1 */
                z->parent->color = BLACK;
                u->color = BLACK;
                g->color = RED;
                z = g;
            } else {
                if (z == z->parent->left) {
                    /* Case 2 */
                    z = z->parent;
                    rotate_right(root, z);
                }
                /* Case 3 */
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rotate_left(root, z->parent->parent);
            }
        }
    }
    /* The root may have been recolored red by Case 1; force it black.
     * This single line also colors the very first inserted node black. */
    (*root)->color = BLACK;
}

/**
 * Replace the subtree rooted at u with the subtree rooted at v: u's
 * parent now points to v, and v->parent is updated. Does not touch v's
 * children. v may be NIL, in which case NIL->parent is set so that
 * delete_fixup can navigate from an empty position.
 */
static void transplant(Node **root, Node *u, Node *v) {
    replace_child(root, u, v);
    v->parent = u->parent;
}

/**
 * Return the minimum (leftmost) node of the subtree rooted at x.
 */
static Node *tree_minimum(Node *x) {
    assert(x != NIL);
    while (x->left != NIL) {
        x = x->left;
    }
    return x;
}

/**
 * Restore the red-black invariants after a black node was removed.
 *
 * Removing a black node leaves one root-to-NIL path one black short.
 * The node x now standing in that path carries an "extra black" (it is
 * conceptually doubly black). This loop pushes that extra black around
 * until it can be absorbed. Let w be x's sibling; the four cases are:
 *
 *   Case 1 (w red): recolor and rotate to make x's sibling black, then
 *     continue with Cases 2-4.
 *   Case 2 (w black, both of w's children black): recolor w red and move
 *     the extra black up to the parent. The only case that propagates.
 *   Case 3 (w black, near child red, far child black): recolor and
 *     rotate to turn it into Case 4.
 *   Case 4 (w black, far child red): recolor and rotate at the parent to
 *     absorb the extra black; this terminates the loop.
 *
 * Rotations occur only in Cases 1, 3, 4, and the worst path 1->3->4
 * gives at most three rotations per delete.
 */
static void delete_fixup(Node **root, Node *x) {
    while (x != *root && x->color == BLACK) {
        if (x == x->parent->left) {
            Node *w = x->parent->right;

            if (w->color == RED) {
                /* Case 1 */
                w->color = BLACK;
                x->parent->color = RED;
                rotate_left(root, x->parent);
                w = x->parent->right;
            }

            if (w->left->color == BLACK && w->right->color == BLACK) {
                /* Case 2 */
                w->color = RED;
                x = x->parent;
            } else {
                if (w->right->color == BLACK) {
                    /* Case 3 */
                    w->left->color = BLACK;
                    w->color = RED;
                    rotate_right(root, w);
                    w = x->parent->right;
                }
                /* Case 4 */
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                rotate_left(root, x->parent);
                x = *root; /* done */
            }
        } else {
            /* Mirror image (x is a right child) */
            Node *w = x->parent->left;

            if (w->color == RED) {
                /* Case 1 */
                w->color = BLACK;
                x->parent->color = RED;
                rotate_right(root, x->parent);
                w = x->parent->left;
            }

            if (w->right->color == BLACK && w->left->color == BLACK) {
                /* Case 2 */
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    /* Case 3 */
                    w->right->color = BLACK;
                    w->color = RED;
                    rotate_left(root, w);
                    w = x->parent->left;
                }
                /* Case 4 */
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rotate_right(root, x->parent);
                x = *root; /* done */
            }
        }
    }
    /* x is either red (absorb the extra black) or the root. */
    x->color = BLACK;
}

/**
 * Locate the node holding key, or NIL if absent.
 */
static Node *find_node(Node *root, int key) {
    while (root != NIL && root->key != key) {
        root = (key < root->key) ? root->left : root->right;
    }
    return root;
}

/* === Public API === */

void rb_insert(Node **root, int key) {
    Node *z = bst_insert(root, key);
    if (z == NIL) {
        return; /* duplicate */
    }
    insert_fixup(root, z);
}

void rb_delete(Node **root, int key) {
    Node *z = find_node(*root, key);
    if (z == NIL) {
        return; /* not present */
    }

    Node *y = z;                     /* node actually spliced out */
    Color y_original_color = y->color;
    Node *x;                         /* node that moves into y's place */

    if (z->left == NIL) {
        x = z->right;
        transplant(root, z, z->right);
    } else if (z->right == NIL) {
        x = z->left;
        transplant(root, z, z->left);
    } else {
        /* Two children: y is z's in-order successor. */
        y = tree_minimum(z->right);
        y_original_color = y->color;
        x = y->right;

        if (y->parent == z) {
            /* y is z's direct right child; x stays under y.
             * Set x->parent explicitly in case x is NIL, so that
             * delete_fixup can navigate from it. */
            x->parent = y;
        } else {
            transplant(root, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(root, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color; /* y inherits z's color/position */
    }

    free(z);

    /* Only the removal of a black node can break invariant 5. */
    if (y_original_color == BLACK) {
        delete_fixup(root, x);
    }
}

bool rb_search(const Node *root, int key) {
    while (root != NIL && root->key != key) {
        root = (key < root->key) ? root->left : root->right;
    }
    return root != NIL;
}

static void free_subtree(Node *node) {
    if (node == NIL) {
        return;
    }
    /* Post-order: free both children before the node itself, so the
     * child pointers are still valid when we recurse. */
    free_subtree(node->left);
    free_subtree(node->right);
    free(node);
}

void rb_destroy(Node **root) {
    free_subtree(*root);
    *root = NIL;
    /* NIL is static storage and is never freed. */
}

/* === Validation (for testing/debugging) === */

/* Invariant 4: a red node has no red child. */
static bool check_red_property(const Node *node) {
    if (node == NIL) {
        return true;
    }
    if (node->color == RED &&
        (node->left->color == RED || node->right->color == RED)) {
        return false;
    }
    return check_red_property(node->left) &&
           check_red_property(node->right);
}

/* Invariant 5: every root-to-NIL path has equal black-height.
 * Returns the black-height, or -1 if a mismatch is found. */
static int black_height(const Node *node) {
    if (node == NIL) {
        return 1; /* count the black NIL */
    }
    int lh = black_height(node->left);
    int rh = black_height(node->right);
    if (lh == -1 || rh == -1 || lh != rh) {
        return -1;
    }
    return lh + (node->color == BLACK ? 1 : 0);
}

/* BST ordering: each key strictly within (min, max). */
static bool check_bst(const Node *node, int min, int max) {
    if (node == NIL) {
        return true;
    }
    if (node->key <= min || node->key >= max) {
        return false;
    }
    return check_bst(node->left, min, node->key) &&
           check_bst(node->right, node->key, max);
}

bool rb_is_valid(const Node *root) {
    if (root != NIL && root->color != BLACK) {
        return false; /* invariant 2 */
    }
    if (!check_red_property(root)) {
        return false; /* invariant 4 */
    }
    if (black_height(root) == -1) {
        return false; /* invariant 5 */
    }
    if (!check_bst(root, INT_MIN, INT_MAX)) {
        return false; /* BST ordering */
    }
    return true;
}

/* === Print (for debugging) === */

static void print_recursive(const Node *node, int depth) {
    if (node == NIL) {
        return;
    }
    print_recursive(node->right, depth + 1);
    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    printf("%d(%c)\n", node->key, node->color == RED ? 'R' : 'B');
    print_recursive(node->left, depth + 1);
}

void rb_print(const Node *root) {
    if (root == NIL) {
        printf("(empty tree)\n");
        return;
    }
    printf("--- RB tree ---\n");
    print_recursive(root, 0);
    printf("---------------\n");
}
