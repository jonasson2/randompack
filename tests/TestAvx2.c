// -*- C -*-
// Test AVX2 path is used when available
#include "test_util.h"
#include "xCheck.h"
#include "randompack_internal.h"

void TestAvx2(void) {
#if defined(BUILD_AVX2) && defined(RANDOMPACK_TEST_HOOKS)
  randompack_rng *rng = randompack_create("x256++simd");
  if (!rng) {
    xCheck(false);
    return;
  }
  if (!rng->cpu_has_avx2 || rng->cpu_has_avx512) {
    randompack_free(rng);
    return;
  }
  randompack_avx2_reset();
  uint64_t x[8];
  bool ok = randompack_uint64(x, LEN(x), 0, rng);
  check_success(ok, rng);
  xCheck(randompack_avx2_used() > 0);
  randompack_free(rng);
#else
  xCheck(true);
#endif
}
