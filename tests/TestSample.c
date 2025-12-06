// -*- C -*-
#include <stdbool.h>

#include "randompack.h"
#include "TestUtil.h"
#include "xCheck.h"

static void test_sample_api(void) {
  int N = 50;
  int K = 10;
  int sample1[K], sample2[K];
  int used[N];
  randompack_rng *r1 = randompack_create("Xorshift", 11);
  randompack_rng *r2 = randompack_create("Xorshift", 11);
  randompack_sample(sample1, N, K, r1);
  randompack_sample(sample2, N, K, r2);
  for (int i = 0; i < N; i++) used[i] = 0;
  for (int i = 0; i < K; i++) {
    int v = sample1[i];
    xCheck(v >= 0 && v < N);
    xCheck(!used[v]);
    used[v] = 1;
    xCheck(sample1[i] == sample2[i]);
  }
  randompack_rng *r3 = randompack_create("Xorshift", 5);
  randompack_sample(0, N, 0, r3);
  randompack_free(r1);
  randompack_free(r2);
  randompack_free(r3);
}

void TestSample(void) {
  test_sample_api();
}
