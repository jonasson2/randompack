// -*- C -*-
// Tests for randompack_norm, N(0,1): determinism, edge cases, and PIT->U01 checks.

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

static void test_basic(char *engine) {
  TEST_DETERMINISM0(engine, double, norm);
  TEST_EDGE_CASES0(engine, double, norm);
  TEST_DETERMINISM0(engine, float, normf);
  TEST_EDGE_CASES0(engine, float, normf);
}

static void test_PIT(char *engine) {
  int N = N_STAT_FAST;
  double *x, *u;
  float *y, *v;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  TEST_ALLOC(y, N);
  TEST_ALLOC(v, N);
  DRAW(engine, 7, randompack_norm(x, N, rng));
  DRAW(engine, 7, randompack_normf(y, N, rng));
  xCheckMsg(check_normal_max(x, N), engine);
  for (int i = 0; i < N; i++) u[i] = normcdf(x[i]);
  for (int i = 0; i < N; i++) v[i] = (float)normcdf(y[i]);
  check_u01_distribution(u, N, "norm", engine);
  check_u01_distributionf(v, N, "normf", engine);
  for (int i = 0; i < N; i++) x[i] = y[i];
  xCheckMsg(check_normal_max(x, N), engine);
  FREE(v);
  FREE(y);
  FREE(u);
  FREE(x);
}

void TestNorm(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e);
  }
}
