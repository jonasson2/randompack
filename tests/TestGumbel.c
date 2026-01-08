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
  float mu_f = 0.0f;
  float beta_f = 1.0f;
  TEST_DETERMINISM2(engine, double, gumbel, mu, beta);
  TEST_EDGE_CASES2(engine, double, gumbel, mu, beta);
  TEST_DETERMINISM2(engine, float, gumbelf, mu_f, beta_f);
  TEST_EDGE_CASES2(engine, float, gumbelf, mu_f, beta_f);
  TEST_ILLEGAL_PARAMS2(double, engine, gumbel, mu, 0.0);
  TEST_ILLEGAL_PARAMS2(double, engine, gumbel, mu, -1.0);
  TEST_ILLEGAL_PARAMS2(float, engine, gumbelf, mu_f, 0.0f);
  TEST_ILLEGAL_PARAMS2(float, engine, gumbelf, mu_f, -1.0f);
}

static void test_PIT(char *engine, double mu, double beta) {
  size_t N = N_STAT_SLOW;
  double *x, *u;
  float *y, *v;
  float mu_f = mu;
  float beta_f = beta;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  TEST_ALLOC(y, N);
  TEST_ALLOC(v, N);
  DRAW(engine, 7, randompack_gumbel(x, N, mu, beta, rng));
  DRAW(engine, 7, randompack_gumbelf(y, N, mu_f, beta_f, rng));
  for (size_t i = 0; i < N; i++)
    u[i] = gumbel_cdf(x[i], mu, beta);
  for (size_t i = 0; i < N; i++)
    v[i] = (float)gumbel_cdf((double)y[i], mu_f, beta_f);
  check_u01_distribution(u, N, "gumbel", engine);
  check_u01_distributionf(v, N, "gumbelf", engine);
  FREE(v);
  FREE(y);
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
