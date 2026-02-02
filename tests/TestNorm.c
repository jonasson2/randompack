// -*- C -*-
// Tests for randompack_norm, N(0,1): max-value sanity checks.

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

// Helper: check max absolute value drawn:
bool check_normal_max(double *x, int n) {
  double q, zlo, zhi, M;
  q = TEST_P_VALUE;
  zlo = probit((1 + pow(q/2, 1.0/n))/2);
  zhi = -probit(q/4/n);
  M = fmax(maxvd(x, n), -minvd(x, n));
  printD("normal max observed", M);
  printD("  lower confidence bound", zlo);
  printD("  upper confidence bound", zhi);
  return (zlo <= M && M <= zhi);
}

static void test_max(char *engine) {
  int N = N_STAT_FAST;
  double *x;
  float *y;
  TEST_ALLOC(x, N);
  TEST_ALLOC(y, N);
  DRAW(engine, 7, randompack_norm(x, N, rng));
  DRAW(engine, 7, randompack_normf(y, N, rng));
  xCheckMsg(check_normal_max(x, N), engine);
  for (int i = 0; i < N; i++) x[i] = y[i];
  xCheckMsg(check_normal_max(x, N), engine);
  FREE(y);
  FREE(x);
}

void TestNorm(void) {
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    char *e = engines[i];
    test_max(e);
  }
  free_engines(engines, n);
}
