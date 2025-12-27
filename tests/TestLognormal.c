// -*- C -*-
// Tests for randompack_lognormal: determinism, edge cases, and PIT->U01.

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

static double lognormal_cdf(double x, double mu, double sigma) {
  if (x <= 0.0) return 0.0;
  return normcdf((log(x) - mu)/sigma);
}

static void test_basic(char *engine) {
  double mu = -0.7;
  double sigma = 1.3;
  TEST_DETERMINISM2(engine, double, lognormal, mu, sigma);
  TEST_EDGE_CASES2(engine, double, lognormal, mu, sigma);
  TEST_ILLEGAL_PARAMS2(double, engine, lognormal, mu, 0.0);
  TEST_ILLEGAL_PARAMS2(double, engine, lognormal, mu, -1.0);
}

static void test_PIT(char *engine, double mu, double sigma) {
  int N = N_STAT_SLOW;
  double *x, *u;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  DRAW(engine, 42, randompack_lognormal(x, N, mu, sigma, rng));
  TEST_SUPPORT(double, x, N, 0, INFINITY);
  for (int i = 0; i < N; i++) u[i] = lognormal_cdf(x[i], mu, sigma);
  check_u01_distribution(u, N);
  FREE(u);
  FREE(x);
}

void TestLognormal(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 0, 1);
    test_PIT(e, -0.7, 0.4);
    test_PIT(e, 1.2, 1.3);
  }
}
