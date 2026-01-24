// -*- C -*-
// Basic tests for randompack_beta: determinism and PIT->U01 checks.

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

static void test_basic(char *engine) {
  double a = 2, b = 3;
  float a_f = 2.0f, b_f = 3.0f;
  TEST_DETERMINISM2(engine, double, beta, a, b);
  TEST_EDGE_CASES2(engine, double, beta, a, b);
  TEST_DETERMINISM2(engine, float, betaf, a_f, b_f);
  TEST_EDGE_CASES2(engine, float, betaf, a_f, b_f);
  TEST_ILLEGAL_PARAMS2(double, engine, beta, 0, b);
  TEST_ILLEGAL_PARAMS2(double, engine, beta, -1, b);
  TEST_ILLEGAL_PARAMS2(double, engine, beta, a, 0);
  TEST_ILLEGAL_PARAMS2(double, engine, beta, a, -1);
  TEST_ILLEGAL_PARAMS2(float, engine, betaf, 0, b_f);
  TEST_ILLEGAL_PARAMS2(float, engine, betaf, -1, b_f);
  TEST_ILLEGAL_PARAMS2(float, engine, betaf, a_f, 0);
  TEST_ILLEGAL_PARAMS2(float, engine, betaf, a_f, -1);
}

static void test_PIT(char *engine, double a, double b) {
  int N = N_STAT_SLOW;
  double *x, *u;
  float *y;
  float *v;
  float a_f = a;
  float b_f = b;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  TEST_ALLOC(y, N);
  TEST_ALLOC(v, N);
  DRAW(engine, 42, randompack_beta(x, N, a, b, rng));
  DRAW(engine, 42, randompack_betaf(y, N, a_f, b_f, rng));
  TEST_SUPPORT(double, x, N, 0, 1);
  TEST_SUPPORT(float, y, N, 0, 1);
  for (int i = 0; i < N; i++) u[i] = incbet(a, b, x[i]);
  for (int i = 0; i < N; i++)
    v[i] = (float)incbet(a_f, b_f, (double)y[i]);
  check_u01_distribution(u, N, "beta", engine);
  check_u01_distributionf(v, N, "betaf", engine);
  FREE(v);
  FREE(y);
  FREE(u);
  FREE(x);
}

void TestBeta(void) {
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 0.5, 0.5);
    test_PIT(e, 2, 5);
    test_PIT(e, 0.7, 3);
  }
  free_engines(engines, n);
}
