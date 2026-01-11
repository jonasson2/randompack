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
  float scale_f = 1.0f;
  TEST_DETERMINISM1(engine, double, exp, scale);
  TEST_EDGE_CASES1(engine, double, exp, scale);
  TEST_DETERMINISM1(engine, float, expf, scale_f);
  TEST_EDGE_CASES1(engine, float, expf, scale_f);
  TEST_ILLEGAL_PARAMS1(double, engine, exp, 0);
  TEST_ILLEGAL_PARAMS1(double, engine, exp, -1);
  TEST_ILLEGAL_PARAMS1(float, engine, expf, 0);
  TEST_ILLEGAL_PARAMS1(float, engine, expf, -1);
}

static void test_PIT(char *engine, double scale) {
  int N = N_STAT_FAST;
  double *x, *u;
  float *y, *v;
  float scale_f = scale;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  TEST_ALLOC(y, N);
  TEST_ALLOC(v, N);
  DRAW(engine, 7, randompack_exp(x, N, scale, rng));
  DRAW(engine, 7, randompack_expf(y, N, scale_f, rng));
  TEST_SUPPORT(double, x, N, 0, INFINITY);
  TEST_SUPPORT(float, y, N, 0, INFINITY);
  for (int i = 0; i < N; i++) u[i] = 1 - exp(-x[i]/scale);
  for (int i = 0; i < N; i++) {
    float yi = y[i];
    float ui = 1 - expf(-yi/scale_f);
    v[i] = ui;
    if (ui == 0 || ui == 1) {
      printD("TestPIT hit an endpoint, ui", (double)ui);
      printS("  engine", engine);
      printD("  yi", (double)yi);
    }
  }
  // for (int i = 0; i < N; i++) v[i] = 1.0f - expf(-y[i]/scale_f);
  // check_u01_distribution(u, N, "exp", engine);
  check_u01_distributionf(v, N, "expf", engine);
  FREE(v);
  FREE(y);
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
