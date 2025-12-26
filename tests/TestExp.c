// -*- C -*-
// Basic tests for randompack_exp: determinism and PIT->U01 checks.

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

static void test_basic(char *engine) {
  double scale = 1;
  TEST_DETERMINISM1(engine, exp, scale);
  TEST_EDGE_CASES1(engine, exp, scale);
  TEST_ILLEGAL_PARAMS1(engine, exp, 0);
  TEST_ILLEGAL_PARAMS1(engine, exp, -1);
}

static void test_PIT(char *engine, double scale) {
  enum { N = 20000 };
  double *x, *u;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  DRAW(engine, 7, randompack_exp(x, N, scale, rng));
  for (int i = 0; i < N; i++) u[i] = 1 - exp(-x[i]/scale);
  xCheck(check_u01_distribution(u, N));
  FREE(u);
  FREE(x);
}

void TestExp(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 1);
    test_PIT(e, 2);
  }
}
