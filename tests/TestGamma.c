// -*- C -*-
// Basic tests for randompack_gamma: determinism and PIT->U01 checks.

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

static void test_basic(char *engine) {
  double scale = 1;
  double shape = 1;
  float scale_f = 1.0f;
  float shape_f = 1.0f;
  TEST_DETERMINISM2(engine, double, gamma, shape, scale);
  TEST_EDGE_CASES2(engine, double, gamma, shape, scale);
  TEST_DETERMINISM2(engine, float, gammaf, shape_f, scale_f);
  TEST_EDGE_CASES2(engine, float, gammaf, shape_f, scale_f);
  TEST_ILLEGAL_PARAMS2(double, engine, gamma, 0, 1);
  TEST_ILLEGAL_PARAMS2(double, engine, gamma, -1, 1);
  TEST_ILLEGAL_PARAMS2(double, engine, gamma, 0.7, 0);
  TEST_ILLEGAL_PARAMS2(double, engine, gamma, 0.7, -1);
  TEST_ILLEGAL_PARAMS2(float, engine, gammaf, 0, 1);
  TEST_ILLEGAL_PARAMS2(float, engine, gammaf, -1, 1);
  TEST_ILLEGAL_PARAMS2(float, engine, gammaf, 0.7, 0);
  TEST_ILLEGAL_PARAMS2(float, engine, gammaf, 0.7, -1);
}

static void test_PIT(char *engine, double shape, double scale) {
  int N = N_STAT_SLOW;
  double *x, *u;
  float *y, *v;
  float shape_f = shape;
  float scale_f = scale;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  TEST_ALLOC(y, N);
  TEST_ALLOC(v, N);
  DRAW(engine, 42, randompack_gamma(x, N, shape, scale, rng));
  DRAW(engine, 42, randompack_gammaf(y, N, shape_f, scale_f, rng));
  TEST_SUPPORT(double, x, N, 0, INFINITY);
  TEST_SUPPORT(float, y, N, 0, INFINITY);
  for (int i = 0; i < N; i++) u[i] = gamma_cdf(x[i], shape, scale);
  for (int i = 0; i < N; i++)
    v[i] = (float)gamma_cdf((double)y[i], shape_f, scale_f);
  check_u01_distribution(u, N, "gamma", engine);
  check_u01_distributionf(v, N, "gammaf", engine);
  FREE(v);
  FREE(y);
  FREE(u);
  FREE(x);
}

void TestGamma(void) {
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 0.7, 1);
    test_PIT(e, 2.0, 1);
    test_PIT(e, 0.3, 3);
  }
  free_engines(engines, n);
}
