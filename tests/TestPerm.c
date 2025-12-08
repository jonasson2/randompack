// -*- C -*-
#include <stdbool.h>

#include "randompack.h"
#include "TestUtil.h"
#include "xCheck.h"

static void test_perm_api(void) {
  int N = 32;
  int perm[N];
  randompack_rng *rng = randompack_create("xoshiro256++", 77);
  check_rng_clean(rng);
  bool ok = randompack_perm(perm, N, rng);
  check_success(ok, rng);
  xCheck(minv(perm, N) >= 0 && maxv(perm, N) < N);
  xCheck(is_perm_0_to_n_minus1(perm, N));
  randompack_free(rng);
}

void TestPerm(void) {
  test_perm_api();
}
