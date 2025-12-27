// -*- C -*-
// Tests for randompack_norm, N(0,1): determinism, edge cases, and simple stats.
#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

// Helper: check max absolute value drawn:
bool check_normal_max(double *x, int n) {
  // [zlo, zhi] is the (1-q) confidence interval for max|x|; fairly easy to derive
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

static void test_basic(char *engine) {
  TEST_DETERMINISM0(engine, double, norm);
  TEST_EDGE_CASES0(engine, double, norm);
}

static void test_statistics(char *engine) {
  int N = N_STAT_FAST;
  double *x;
  TEST_ALLOC(x, N);
  DRAW(engine, 7, randompack_norm(x, N, rng));
  xCheck(check_meanvar(x, N));
  xCheck(check_skewkurt(x, N, 0, 3));
  xCheck(check_normal_max(x, N));
  FREE(x);
}

void TestNorm(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_statistics(e);
  }
}
