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

// 정답지: 모든 점을 전수조사해서 최근접 찾기 (느리지만 확실)
static void brute_force(double pts[][K], int n, const double q[K],
                        int *best_idx, double *best_sq) {
  *best_sq = -1;
  for (int i = 0; i < n; i++) {
    double s = dist_sq_pub(pts[i], q); // 거리 제곱 (아래 설명)
    if (*best_sq < 0 || s < *best_sq) {
      *best_sq = s;
      *best_idx = i;
    }
  }
}

int main(void) {
  srand(time(NULL)); // 매번 다른 랜덤 (디버깅 땐 고정 seed가 나을 수도)

  const int N = 200;        // 점 개수
  const int TRIALS = 10000; // 질의 횟수
  const double RANGE = 100.0;

  double pts[200][K];

  // 1. 랜덤 점 생성 + k-d 트리에 삽입
  kdnode *root = NULL;
  for (int i = 0; i < N; i++) {
    for (int d = 0; d < K; d++)
      pts[i][d] = (double)rand() / RAND_MAX * RANGE;
    root = kd_insert(root, pts[i]);
  }

  // 2. 랜덤 질의 TRIALS번, k-d vs brute force 대조
  int fails = 0;
  for (int t = 0; t < TRIALS; t++) {
    double q[K];
    for (int d = 0; d < K; d++)
      q[d] = (double)rand() / RAND_MAX * RANGE;

    kdnode *kd = kd_nearest(root, q); // k-d 답
    int bf_idx;
    double bf_sq;
    brute_force(pts, N, q, &bf_idx, &bf_sq); // 정답지

    double kd_sq = dist_sq_pub(kd->point, q);
    // 거리가 같으면 OK (동률 점 있을 수 있으니 "같은 노드"가 아니라 "같은
    // 거리"로 비교!)
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