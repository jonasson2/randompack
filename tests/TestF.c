// -*- C -*-
// Tests for randompack_f (F distribution): determinism, edge cases, and PIT->U01.

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

static double f_cdf(double x, double nu1, double nu2) {
  if (x <= 0.0) return 0.0;
  double z = (nu1*x)/(nu1*x + nu2);
  return incbet(0.5*nu1, 0.5*nu2, z);
}

static void test_basic(char *engine) {
  double nu1 = 5;
  double nu2 = 7;
  TEST_DETERMINISM2(engine, double, f, nu1, nu2);
  TEST_EDGE_CASES2(engine, double, f, nu1, nu2);
  TEST_ILLEGAL_PARAMS2(double, engine, f, 0, 1);
  TEST_ILLEGAL_PARAMS2(double, engine, f, -1, 1);
  TEST_ILLEGAL_PARAMS2(double, engine, f, 1, 0);
  TEST_ILLEGAL_PARAMS2(double, engine, f, 1, -1);
}

static void test_PIT(char *engine, double nu1, double nu2) {
  int N = N_STAT_SLOW;
  double *x, *u;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  DRAW(engine, 42, randompack_f(x, N, nu1, nu2, rng));
  TEST_SUPPORT(double, x, N, 0, INFINITY);
  for (int i = 0; i < N; i++) u[i] = f_cdf(x[i], nu1, nu2);
  check_u01_distribution(u, N);
  FREE(u);
  FREE(x);
}

void TestF(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 1, 1);
    test_PIT(e, 5, 7);
    test_PIT(e, 30, 40);
  }
}
