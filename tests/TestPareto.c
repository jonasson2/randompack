// -*- C -*-
// Basic tests for randompack_pareto: determinism, edge cases,
// illegal parameters, support, and PIT->U01 checks.

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

static inline double pareto_cdf(double x, double xm, double alpha) {
  if (x < xm) return 0;
  return 1 - pow(xm/x, alpha);
}

static void test_basic(char *engine) {
  double xm = 1;
  double alpha = 2;
  TEST_DETERMINISM2(engine, double, pareto, xm, alpha);
  TEST_EDGE_CASES2(engine, double, pareto, xm, alpha);
  TEST_ILLEGAL_PARAMS2(double, engine, pareto, 0, 2);
  TEST_ILLEGAL_PARAMS2(double, engine, pareto, -1, 2);
  TEST_ILLEGAL_PARAMS2(double, engine, pareto, 1, 0);
  TEST_ILLEGAL_PARAMS2(double, engine, pareto, 1, -1);
}

static void test_PIT(char *engine, double xm, double alpha) {
  int N = N_STAT_SLOW;
  double *x, *u;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  DRAW(engine, 42, randompack_pareto(x, N, xm, alpha, rng));
  TEST_SUPPORT(double, x, N, xm, INFINITY);
  for (int i = 0; i < N; i++) u[i] = pareto_cdf(x[i], xm, alpha);
  check_u01_distribution(u, N);
  FREE(u);
  FREE(x);
}

void TestPareto(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 1, 2);
    test_PIT(e, 1, 1.5);
    test_PIT(e, 2, 0.7);
  }
}
