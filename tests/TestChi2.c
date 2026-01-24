// -*- C -*-
// Basic tests for randompack_chi2: determinism and PIT->U01 checks.

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

static void test_basic(char *engine) {
  double nu = 2;
  float nu_f = 2.0f;
  TEST_DETERMINISM1(engine, double, chi2, nu);
  TEST_EDGE_CASES1(engine, double, chi2, nu);
  TEST_DETERMINISM1(engine, float, chi2f, nu_f);
  TEST_EDGE_CASES1(engine, float, chi2f, nu_f);
  TEST_ILLEGAL_PARAMS1(double, engine, chi2, 0);
  TEST_ILLEGAL_PARAMS1(double, engine, chi2, -1);
  TEST_ILLEGAL_PARAMS1(float, engine, chi2f, 0);
  TEST_ILLEGAL_PARAMS1(float, engine, chi2f, -1);
}

static void test_PIT(char *engine, int nu) {
  int N = N_STAT_SLOW;
  double *x, *u;
  float *y, *v;
  float nu_f = nu;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  TEST_ALLOC(y, N);
  TEST_ALLOC(v, N);
  DRAW(engine, 42, randompack_chi2(x, N, nu, rng));
  DRAW(engine, 42, randompack_chi2f(y, N, nu_f, rng));
  TEST_SUPPORT(double, x, N, 0, INFINITY);
  TEST_SUPPORT(float, y, N, 0, INFINITY);
  for (int i = 0; i < N; i++) u[i] = chi2_cdf(x[i], nu);
  for (int i = 0; i < N; i++)
    v[i] = (float)chi2_cdf((double)y[i], nu_f);
  check_u01_distribution(u, N, "chi2", engine);
  check_u01_distributionf(v, N, "chi2f", engine);
  FREE(v);
  FREE(y);
  FREE(u);
  FREE(x);
}

void TestChi2(void) {
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 1);
    test_PIT(e, 2);
    test_PIT(e, 10);
  }
  free_engines(engines, n);
}
