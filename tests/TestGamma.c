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
  TEST_DETERMINISM2(engine, gamma, shape, scale);
  TEST_EDGE_CASES2(engine, gamma, shape, scale);
  TEST_ILLEGAL_PARAMS2(engine, gamma, 0, 1);
  TEST_ILLEGAL_PARAMS2(engine, gamma, -1, 1);
  TEST_ILLEGAL_PARAMS2(engine, gamma, 0.7, 0);
  TEST_ILLEGAL_PARAMS2(engine, gamma, 0.7, -1);
}

static void test_PIT(char *engine, double shape, double scale) {
  enum { N = 20000 };
  double *x, *u;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  DRAW(engine, 42, randompack_gamma(x, N, shape, scale, rng));
  for (int i = 0; i < N; i++) u[i] = gamma_cdf(x[i], shape, scale);
  xCheck(check_u01_distribution(u, N));
  FREE(u);
  FREE(x);
}

void TestGamma(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 0.7, 1);
    test_PIT(e, 2.0, 1);
    test_PIT(e, 0.3, 3);
  }
}
