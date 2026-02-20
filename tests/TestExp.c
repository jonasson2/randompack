// -*- C -*-
// Tests for randompack_exp, Exp(1): determinism, edge cases, and PIT->U01.

#include <math.h>
#include <stdbool.h>

#include "test_util.h"
#include "test_cdfs.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

static void test_basic(char *engine) {
  double a = 1;
  TEST_DETERMINISM1(engine, double, exp, a);
  TEST_EDGE_CASES1(engine, double, exp, a);
  TEST_ILLEGAL_PARAMS1(double, engine, exp, 0);
  TEST_ILLEGAL_PARAMS1(double, engine, exp, -1);
}

static void test_PIT(char *engine, double scale) {
  int N = N_STAT_FAST;
  double *x, *u;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  DRAW(engine, 42, randompack_exp(x, N, scale, rng));
  TEST_SUPPORT(double, x, N, 0, INFINITY);
  for (int i = 0; i < N; i++) u[i] = test_cdf_exp(x[i], scale);
  check_u01_distribution(u, N, "exp", engine);
  FREE(u);
  FREE(x);
}

void TestExp(void) {
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 1);
    test_PIT(e, 2);
  }
  free_engines(engines, n);
}

void TestExpx(char *engine) {
  test_basic(engine);
  test_PIT(engine, 1);
  test_PIT(engine, 2);
}
