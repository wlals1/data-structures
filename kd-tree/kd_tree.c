#include "kd_tree.h"
#include <stdlib.h>

static kdnode *create_node(const double point[K]) {
  kdnode *n = calloc(1, sizeof(kdnode));
  if (!n)
    return NULL;
  for (int i = 0; i < K; i++)
    n->point[i] = point[i];
  return n;
}

static kdnode *insert_helper(kdnode *root, const double point[K], int depth) {
  if (root == NULL)
    return create_node(point);
  int axis = depth % K;
  if (point[axis] < root->point[axis]) {
    root->left = insert_helper(root->left, point, depth + 1);
  } else {
    root->right = insert_helper(root->right, point, depth + 1);
  }
  return root;
}

kdnode *kd_insert(kdnode *root, const double point[K]) {
  return insert_helper(root, point, 0);
}

static double dist_sq(const double a[K], const double b[K]) {
  double sum = 0;
  for (int i = 0; i < K; i++) {
    double d = a[i] - b[i];
    sum += d * d;
  }
  return sum;
}

static void nearest_helper(kdnode *root, const double query[K], int depth,
                           kdnode **best, double *best_sq) {
  if (root == NULL)
    return;

  double d = dist_sq(root->point, query);
  if (d < *best_sq) {
    *best_sq = d;
    *best = root; // ← *best = root (not best = &root)
  }

  int axis = depth % K;
  kdnode *near, *far;
  if (query[axis] < root->point[axis]) {
    near = root->left;
    far = root->right;
  } else {
    near = root->right;
    far = root->left;
  }

  nearest_helper(near, query, depth + 1, best, best_sq);

  double axis_diff = query[axis] - root->point[axis];
  if (axis_diff * axis_diff < *best_sq)
    nearest_helper(far, query, depth + 1, best, best_sq);
}

kdnode *kd_nearest(kdnode *root, const double query[K]) {
  if (root == NULL)
    return NULL;
  kdnode *best = root;
  double best_sq = dist_sq(query, root->point);
  nearest_helper(root, query, 0, &best, &best_sq);
  return best;
}

void kd_free(kdnode *root) {
  if (root == NULL)
    return;
  kd_free(root->left);
  kd_free(root->right);
  free(root);
}