// -*- C -*-
#include <stdbool.h>
#include <stdlib.h>

#include "allocate.h"
#include "ExtraUtil.h"
#include "randompack.h"
#include "xCheck.h"

static void test_randomize_changes_stream(void) {
  const int N = 128;
  double *a, *b, *c;
  allocate(a, N);
  allocate(b, N);
  allocate(c, N);

  randompack_rng *r1 = randompack_create("Xorshift", 42);
  randompack_u01(a, N, r1);

  randompack_rng *r2 = randompack_create("Xorshift", 0);
  randompack_u01(b, N, r2);

  randompack_rng *r3 = randompack_create("Xorshift", -42);
  randompack_u01(c, N, r3);

  xCheck(!almostEqual(a, b, N));
  xCheck(!almostEqual(a, c, N));
  xCheck(!almostEqual(b, c, N));

  randompack_free(r1);
  randompack_free(r2);
  randompack_free(r3);
  freem(a);
  freem(b);
  freem(c);
}

void TestRandomize(void) {
  test_randomize_changes_stream();
}
