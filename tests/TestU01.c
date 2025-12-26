// -*- C -*-
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "TestUtil.h"
#include "randompack_config.h"
#include "printX.h"
#include "randompack.h"
#include "xCheck.h"

// Different seeds => different randoms, same seeds => same randoms
// Edge cases: zero-length, null buffer/rng, and bad bounds, max span.
static void test_basic(char *engine) {
  TEST_DETERMINISM0(engine, u01);
  TEST_EDGE_CASES0(engine, u01);
}

static void test_statistics(char *engine) {
  // Test moments, min/max, and balanced bins for U01
  int N = N_statistics;
  double *x;
  TEST_ALLOC(x, N);
  DRAW(engine, 42, randompack_u01(x, N, rng));
  xCheck(check_u01_distribution(x, N));
  FREE(x);
}

void TestU01(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_basic(e);
    test_statistics(e);
  }
}
