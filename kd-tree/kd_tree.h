#ifndef KD_TREE_H
#define KD_TREE_H
#define K 2

typedef struct kdnode {
  double point[K];
  struct kdnode *left, *right;
} kdnode;

kdnode *kd_insert(kdnode *root, const double point[K]);
kdnode *kd_nearest(kdnode *root, const double query[K]);
void kd_free(kdnode *root);

#endif // KD_TREE_H