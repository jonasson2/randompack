// -*- C -*-
#include <stdbool.h>

#include "randompack.h"
#include "TestUtil.h"
#include "xCheck.h"

static void test_perm_api(void) {
  int N = 32;
  int perm[N];
  randompack_rng *rng = randompack_create("Xorshift", 77);
  randompack_perm(perm, N, rng);
  xCheck(minv(perm, N) >= 0 && maxv(perm, N) < N);
  xCheck(is_perm_0_to_n_minus1(perm, N));
  randompack_free(rng);
}

void TestPerm(void) {
  test_perm_api();
}
