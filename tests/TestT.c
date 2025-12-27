// -*- C -*-
// Tests for randompack_t (Student t): determinism, edge cases, and PIT->U01.

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

// Student-t CDF via incomplete beta.
static double t_cdf(double x, double nu) {
  double t2 = x*x;
  double z = nu/(nu + t2);
  double p = 0.5*incbet(0.5*nu, 0.5, z);
  return (x >= 0) ? 1 - p : p;
}

static void test_basic(char *engine) {
  double nu = 5;
  TEST_DETERMINISM1(engine, double, t, nu);
  TEST_EDGE_CASES1(engine, double, t, nu);
  TEST_ILLEGAL_PARAMS1(double, engine, t, 0);
  TEST_ILLEGAL_PARAMS1(double, engine, t, -1);
}

static void test_PIT(char *engine, double nu) {
  int N = N_STAT_SLOW;
  double *x, *u;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  DRAW(engine, 42, randompack_t(x, N, nu, rng));
  for (int i = 0; i < N; i++)
    u[i] = t_cdf(x[i], nu);
  check_u01_distribution(u, N);
  FREE(u);
  FREE(x);
}

void TestT(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_PIT(e, 1);    // Cauchy
    test_PIT(e, 5);    // heavy tails
    test_PIT(e, 30);   // near normal
  }
}
