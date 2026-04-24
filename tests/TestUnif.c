// -*- C -*-
// Tests for randompack_u01 and randompack_unif, U(a,b): determinism, edge cases,
// and PIT->U01.

#include <math.h>
#include <stdbool.h>

#include "test_util.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

static void test_u01_basic(char *engine) {
  TEST_DETERMINISM0(engine, double, u01);
  TEST_EDGE_CASES0(engine, double, u01);
  TEST_DETERMINISM0(engine, float, u01f);
  TEST_EDGE_CASES0(engine, float, u01f);
}

static void test_u01_statistics(char *engine) {
  int N = N_STAT_FAST;
  double *x;
  float *y;
  TEST_ALLOC(x, N);
  TEST_ALLOC(y, N);
  DRAW(engine, 42, randompack_u01(x, N, rng));
  DRAW(engine, 42, randompack_u01f(y, N, rng));
  TEST_SUPPORT(double, x, N, 0, 1);
  TEST_SUPPORT(float, y, N, 0, 1);
  check_u01_distribution(x, N, "u01", engine);
  check_u01_distributionf(y, N, "u01f", engine);
  FREE(y);
  FREE(x);
}

static void test_basic(char *engine) {
  double a = -2;
  double b = 3;
  float a_f = -2;
  float b_f = 3;
  TEST_DETERMINISM2(engine, double, unif, a, b);
  TEST_EDGE_CASES2(engine, double, unif, a, b);
  TEST_ILLEGAL_PARAMS2(double, engine, unif, 0, 0);
  TEST_ILLEGAL_PARAMS2(double, engine, unif, 1, 0);
  TEST_DETERMINISM2(engine, float, uniff, a_f, b_f);
  TEST_EDGE_CASES2(engine, float, uniff, a_f, b_f);
  TEST_ILLEGAL_PARAMS2(float, engine, uniff, 0, 0);
  TEST_ILLEGAL_PARAMS2(float, engine, uniff, 1, 0);
}

static void test_PIT(char *engine, double a, double b) {
  int N = N_STAT_FAST;
  double *x, *u;
  float *y, *v;
  float a_f = (float)a;
  float b_f = (float)b;
  float w_f = b_f - a_f;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  TEST_ALLOC(y, N);
  TEST_ALLOC(v, N);
  DRAW(engine, 42, randompack_unif(x, N, a, b, rng));
  DRAW(engine, 42, randompack_uniff(y, N, a_f, b_f, rng));
  TEST_SUPPORT(double, x, N, a, b);
  TEST_SUPPORT(float, y, N, a_f, b_f);
  double w = b - a;
  for (int i = 0; i < N; i++) u[i] = (x[i] - a)/w;
  for (int i = 0; i < N; i++) v[i] = (y[i] - a_f)/w_f;
  check_u01_distribution(u, N, "unif", engine);
  check_u01_distributionf(v, N, "uniff", engine);
  FREE(v);
  FREE(y);
  FREE(u);
  FREE(x);
}

void TestUnifx(char *engine) {
  test_u01_basic(engine);
  test_u01_statistics(engine);
  test_basic(engine);
  test_PIT(engine, -2, 3);
  test_PIT(engine, 0, 1);
}

void TestUnif(void) {
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) TestUnifx(engines[i]);
  free_engines(engines, n);
}
