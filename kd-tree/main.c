#include "kd_tree.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static double dist_sq_pub(const double a[K], const double b[K]) {
  double sum = 0;
  for (int i = 0; i < K; i++) {
    double d = a[i] - b[i];
    sum += d * d;
  }
  return sum;
}

static void brute_force(double pts[][K], int n, const double q[K],
                        int *best_idx, double *best_sq) {
  *best_sq = -1;
  for (int i = 0; i < n; i++) {
    double s = dist_sq_pub(pts[i], q);
    if (*best_sq < 0 || s < *best_sq) {
      *best_sq = s;
      *best_idx = i;
    }
  }
}

int main(void) {
  srand(time(NULL));

  const int N = 200;
  const int TRIALS = 10000;
  const double RANGE = 100.0;

  double pts[200][K];

  kdnode *root = NULL;
  for (int i = 0; i < N; i++) {
    for (int d = 0; d < K; d++)
      pts[i][d] = (double)rand() / RAND_MAX * RANGE;
    root = kd_insert(root, pts[i]);
  }

  int fails = 0;
  for (int t = 0; t < TRIALS; t++) {
    double q[K];
    for (int d = 0; d < K; d++)
      q[d] = (double)rand() / RAND_MAX * RANGE;

    kdnode *kd = kd_nearest(root, q);
    int bf_idx;
    double bf_sq;
    brute_force(pts, N, q, &bf_idx, &bf_sq);

    double kd_sq = dist_sq_pub(kd->point, q);
    if (kd_sq != bf_sq) {
      printf("FAIL trial %d: q=(%.2f,%.2f) kd_sq=%.4f bf_sq=%.4f\n", t, q[0],
             q[1], kd_sq, bf_sq);
      fails++;
    }
  }

  printf("%d trials, %d fails\n", TRIALS, fails);
  kd_free(root);
  return 0;
}