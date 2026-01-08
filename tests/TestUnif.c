// -*- C -*-
// Tests for randompack_unif, U(a,b): determinism, edge cases, and PIT->U01.

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

static void test_basic(char *engine) {
  double a = -2;
  double b = 3;
  float a_f = -2.0f;
  float b_f = 3.0f;
  TEST_DETERMINISM2(engine, double, unif, a, b);
  TEST_EDGE_CASES2(engine, double, unif, a, b);
  TEST_DETERMINISM2(engine, float, uniff, a_f, b_f);
  TEST_EDGE_CASES2(engine, float, uniff, a_f, b_f);
  TEST_ILLEGAL_PARAMS2(double, engine, unif, 0, 0);
  TEST_ILLEGAL_PARAMS2(double, engine, unif, 1, 0);
  TEST_ILLEGAL_PARAMS2(float, engine, uniff, 0, 0);
  TEST_ILLEGAL_PARAMS2(float, engine, uniff, 1, 0);
}

static void test_PIT(char *engine, double a, double b) {
  int N = N_STAT_FAST;
  double *x, *u;
  float *y, *v;
  float a_f = a;
  float b_f = b;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  TEST_ALLOC(y, N);
  TEST_ALLOC(v, N);
  DRAW(engine, 42, randompack_unif(x, N, a, b, rng));
  DRAW(engine, 42, randompack_uniff(y, N, a_f, b_f, rng));
  TEST_SUPPORT(double, x, N, a, b);
  TEST_SUPPORT(float, y, N, a_f, b_f);
  double w = b - a;
  float w_f = b_f - a_f;
  for (int i = 0; i < N; i++) u[i] = (x[i] - a)/w;
  for (int i = 0; i < N; i++) v[i] = (y[i] - a_f)/w_f;
  check_u01_distribution(u, N, "unif", engine);
  check_u01_distributionf(v, N, "uniff", engine);
  FREE(v);
  FREE(y);
  FREE(u);
  FREE(x);
}

void TestUnif(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, -2, 3);
    test_PIT(e, 0, 1);
  }
}
