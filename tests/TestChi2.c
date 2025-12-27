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
  TEST_DETERMINISM1(engine, double, chi2, nu);
  TEST_EDGE_CASES1(engine, double, chi2, nu);
  TEST_ILLEGAL_PARAMS1(double, engine, chi2, 0);
  TEST_ILLEGAL_PARAMS1(double, engine, chi2, -1);
}

static void test_PIT(char *engine, int nu) {
  int N = N_STAT_SLOW;
  double *x, *u;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  DRAW(engine, 42, randompack_chi2(x, N, nu, rng));
  TEST_SUPPORT(double, x, N, 0, INFINITY);
  for (int i = 0; i < N; i++) u[i] = chi2_cdf(x[i], nu);
  check_u01_distribution(u, N);
  FREE(u);
  FREE(x);
}

void TestChi2(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 1);
    test_PIT(e, 2);
    test_PIT(e, 10);
  }
}
