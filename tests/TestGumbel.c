// -*- C -*-
// Tests for randompack_gumbel: determinism, edge cases, and PIT->U01 checks.

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

static double gumbel_cdf(double x, double mu, double beta) {
  return exp(-exp(-(x - mu)/beta));
}

static void test_basic(char *engine) {
  double mu = 0.0;
  double beta = 1.0;
  TEST_DETERMINISM2(engine, double, gumbel, mu, beta);
  TEST_EDGE_CASES2(engine, double, gumbel, mu, beta);
  TEST_ILLEGAL_PARAMS2(double, engine, gumbel, mu, 0.0);
  TEST_ILLEGAL_PARAMS2(double, engine, gumbel, mu, -1.0);
}

static void test_PIT(char *engine, double mu, double beta) {
  size_t N = N_STAT_SLOW;
  double *x, *u;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  DRAW(engine, 7, randompack_gumbel(x, N, mu, beta, rng));
  for (size_t i = 0; i < N; i++)
    u[i] = gumbel_cdf(x[i], mu, beta);
  check_u01_distribution(u, N);
  FREE(u);
  FREE(x);
}

void TestGumbel(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 0.0, 1.0);
    test_PIT(e, 2.0, 0.5);
  }
}
