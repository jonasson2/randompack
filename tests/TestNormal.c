// -*- C -*-
// Tests for randompack_normal, N(mu,sigma): determinism, edge cases, and PIT->U01 checks.

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

static void test_basic(char *engine) {
  double mu = 0;
  double sigma = 1;
  float mu_f = 0.0f;
  float sigma_f = 1.0f;
  TEST_DETERMINISM2(engine, double, normal, mu, sigma);
  TEST_EDGE_CASES2(engine, double, normal, mu, sigma);
  TEST_DETERMINISM2(engine, float, normalf, mu_f, sigma_f);
  TEST_EDGE_CASES2(engine, float, normalf, mu_f, sigma_f);
  TEST_ILLEGAL_PARAMS2(double, engine, normal, mu, 0);
  TEST_ILLEGAL_PARAMS2(double, engine, normal, mu, -1);
  TEST_ILLEGAL_PARAMS2(float, engine, normalf, mu_f, 0);
  TEST_ILLEGAL_PARAMS2(float, engine, normalf, mu_f, -1);
}

static void test_PIT(char *engine, double mu, double sigma) {
  int N = N_STAT_FAST;
  double *x, *u;
  float *y, *v;
  float mu_f = mu;
  float sigma_f = sigma;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  TEST_ALLOC(y, N);
  TEST_ALLOC(v, N);
  DRAW(engine, 42, randompack_normal(x, N, mu, sigma, rng));
  DRAW(engine, 42, randompack_normalf(y, N, mu_f, sigma_f, rng));
  for (int i = 0; i < N; i++) u[i] = normcdf((x[i] - mu)/sigma);
  for (int i = 0; i < N; i++)
    v[i] = (float)normcdf(((double)y[i] - mu_f)/sigma_f);
  check_u01_distribution(u, N);
  check_u01_distributionf(v, N);
  FREE(v);
  FREE(y);
  FREE(u);
  FREE(x);
}

void TestNormal(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 0, 1);
    test_PIT(e, 1, 2);
    test_PIT(e, -3, 0.5);
  }
}
