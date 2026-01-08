// -*- C -*-
// Tests for randompack_weibull: determinism, edge cases, and PIT->U01.

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

static double weibull_cdf(double x, double shape, double scale) {
  if (x <= 0.0) return 0.0;
  return 1.0 - exp(-pow(x/scale, shape));
}

static void test_basic(char *engine) {
  double shape = 1.5;
  double scale = 2;
  float shape_f = 1.5f;
  float scale_f = 2.0f;
  TEST_DETERMINISM2(engine, double, weibull, shape, scale);
  TEST_EDGE_CASES2(engine, double, weibull, shape, scale);
  TEST_DETERMINISM2(engine, float, weibullf, shape_f, scale_f);
  TEST_EDGE_CASES2(engine, float, weibullf, shape_f, scale_f);
  TEST_ILLEGAL_PARAMS2(double, engine, weibull, 0, 1);
  TEST_ILLEGAL_PARAMS2(double, engine, weibull, -1, 1);
  TEST_ILLEGAL_PARAMS2(double, engine, weibull, 1, 0);
  TEST_ILLEGAL_PARAMS2(double, engine, weibull, 1, -1);
  TEST_ILLEGAL_PARAMS2(float, engine, weibullf, 0, 1);
  TEST_ILLEGAL_PARAMS2(float, engine, weibullf, -1, 1);
  TEST_ILLEGAL_PARAMS2(float, engine, weibullf, 1, 0);
  TEST_ILLEGAL_PARAMS2(float, engine, weibullf, 1, -1);
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
  DRAW(engine, 42, randompack_weibull(x, N, shape, scale, rng));
  DRAW(engine, 42, randompack_weibullf(y, N, shape_f, scale_f, rng));
  TEST_SUPPORT(double, x, N, 0, INFINITY);
  TEST_SUPPORT(float, y, N, 0, INFINITY);
  for (int i = 0; i < N; i++) u[i] = weibull_cdf(x[i], shape, scale);
  for (int i = 0; i < N; i++)
    v[i] = (float)weibull_cdf((double)y[i], shape_f, scale_f);
  check_u01_distribution(u, N, "weibull", engine);
  check_u01_distributionf(v, N, "weibullf", engine);
  FREE(v);
  FREE(y);
  FREE(u);
  FREE(x);
}

void TestWeibull(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 1, 1);     // reduces to Exp(1)
    test_PIT(e, 0.7, 2);
    test_PIT(e, 3, 0.5);
  }
}
