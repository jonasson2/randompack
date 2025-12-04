// -*- C -*-
#include <stdbool.h>

#include "randompack.h"
#include "TestRandomHelpers.h"
#include "xCheck.h"

#define RUN_TEST(x)        \
  do {                     \
    xCheckAddMsg(#x);      \
    test_##x();            \
  } while (0)

static void test_perm_api(void) {
  const int N = 32;
  int perm[N];
  randompack_rng *rng = randompack_create("Xorshift", 77);
  randompack_perm(perm, N, rng);
  xCheck(min_intv(perm, N) >= 0 && max_intv(perm, N) < N);
  xCheck(is_perm_0_to_n_minus1(perm, N));
  randompack_free(rng);
}

void TestRandomPerm(void) {
  RUN_TEST(perm_api);
}
