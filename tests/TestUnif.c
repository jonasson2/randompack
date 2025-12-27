// -*- C -*-
// Tests for randompack_unif, U(a,b): determinism, edge cases, and PIT->U01.

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

static void test_basic(char *engine) {
  double a = -2;
  double b = 3;
  TEST_DETERMINISM2(engine, double, unif, a, b);
  TEST_EDGE_CASES2(engine, double, unif, a, b);
  TEST_ILLEGAL_PARAMS2(double, engine, unif, 0, 0);
  TEST_ILLEGAL_PARAMS2(double, engine, unif, 1, 0);
}

static void test_PIT(char *engine, double a, double b) {
  int N = N_STAT_FAST;
  double *x, *u;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  DRAW(engine, 42, randompack_unif(x, N, a, b, rng));
  TEST_SUPPORT(double, x, N, a, b);
  double w = b - a;
  for (int i = 0; i < N; i++) u[i] = (x[i] - a)/w;
  check_u01_distribution(u, N);
  FREE(u);
  FREE(x);
}

void TestUnif(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, -2, 3);
    test_PIT(e, 0, 1);
  }
}
