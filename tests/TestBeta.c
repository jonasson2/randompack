// -*- C -*-
// Basic tests for randompack_beta: determinism and PIT->U01 checks.

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

static void test_basic(char *engine) {
  double a = 2, b = 3;
  TEST_DETERMINISM2(engine, double, beta, a, b);
  TEST_EDGE_CASES2(engine, double, beta, a, b);
  TEST_ILLEGAL_PARAMS2(double, engine, beta, 0, b);
  TEST_ILLEGAL_PARAMS2(double, engine, beta, -1, b);
  TEST_ILLEGAL_PARAMS2(double, engine, beta, a, 0);
  TEST_ILLEGAL_PARAMS2(double, engine, beta, a, -1);
}

static void test_PIT(char *engine, double a, double b) {
  int N = N_STAT_SLOW;
  double *x, *u;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  DRAW(engine, 42, randompack_beta(x, N, a, b, rng));
  TEST_SUPPORT(double, x, N, 0, 1);
  for (int i = 0; i < N; i++) u[i] = incbet(a, b, x[i]);
  check_u01_distribution(u, N);
  FREE(u);
  FREE(x);
}

void TestBeta(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 0.5, 0.5);
    test_PIT(e, 2, 5);
    test_PIT(e, 0.7, 3);
  }
}
