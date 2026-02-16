// -*- C -*-
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "test_util.h"
#include "randompack_config.h"
#include "printX.h"
#include "randompack.h"
#include "xCheck.h"

// Different seeds => different randoms, same seeds => same randoms
// Edge cases: zero-length, null buffer/rng, and bad bounds, max span.
static void test_basic(char *engine) {
  TEST_DETERMINISM0(engine, double, u01);
  TEST_EDGE_CASES0(engine, double, u01);
  TEST_DETERMINISM0(engine, float, u01f);
  TEST_EDGE_CASES0(engine, float, u01f);
}

static void test_statistics(char *engine) {
  // Test moments, min/max, and balanced bins for U01
  int N = N_STAT_FAST;
  double *x;
  float *y;
  TEST_ALLOC(x, N);
  TEST_ALLOC(y, N);
  DRAW(engine, 42, randompack_u01(x, N, rng));
  DRAW(engine, 42, randompack_u01f(y, N, rng));
  TEST_SUPPORT(double, x, N, 0, 1);
  TEST_SUPPORT(float, y, N, 0, 1);
  check_u01_distribution(x, N, "u01", engine);
  check_u01_distributionf(y, N, "u01f", engine);
  FREE(y);
  FREE(x);
}

void TestU01(void) {
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    char *e = engines[i];
    test_basic(e);
    test_statistics(e);
  }
  free_engines(engines, n);
}
