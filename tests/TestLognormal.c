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
  float mu_f = -0.7f;
  float sigma_f = 1.3f;
  TEST_DETERMINISM2(engine, double, lognormal, mu, sigma);
  TEST_EDGE_CASES2(engine, double, lognormal, mu, sigma);
  TEST_DETERMINISM2(engine, float, lognormalf, mu_f, sigma_f);
  TEST_EDGE_CASES2(engine, float, lognormalf, mu_f, sigma_f);
  TEST_ILLEGAL_PARAMS2(double, engine, lognormal, mu, 0.0);
  TEST_ILLEGAL_PARAMS2(double, engine, lognormal, mu, -1.0);
  TEST_ILLEGAL_PARAMS2(float, engine, lognormalf, mu_f, 0.0f);
  TEST_ILLEGAL_PARAMS2(float, engine, lognormalf, mu_f, -1.0f);
}

static void test_PIT(char *engine, double mu, double sigma) {
  int N = N_STAT_SLOW;
  double *x, *u;
  float *y, *v;
  float mu_f = mu;
  float sigma_f = sigma;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  TEST_ALLOC(y, N);
  TEST_ALLOC(v, N);
  DRAW(engine, 42, randompack_lognormal(x, N, mu, sigma, rng));
  DRAW(engine, 42, randompack_lognormalf(y, N, mu_f, sigma_f, rng));
  TEST_SUPPORT(double, x, N, 0, INFINITY);
  TEST_SUPPORT(float, y, N, 0, INFINITY);
  for (int i = 0; i < N; i++) u[i] = lognormal_cdf(x[i], mu, sigma);
  for (int i = 0; i < N; i++)
    v[i] = (float)lognormal_cdf((double)y[i], mu_f, sigma_f);
  check_u01_distribution(u, N, "lognormal", engine);
  check_u01_distributionf(v, N, "lognormalf", engine);
  FREE(v);
  FREE(y);
  FREE(u);
  FREE(x);
}

void TestLognormal(void) {
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 0, 1);
    test_PIT(e, -0.7, 0.4);
    test_PIT(e, 1.2, 1.3);
  }
  free_engines(engines, n);
}
