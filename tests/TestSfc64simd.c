// -*- C -*-
// Checking sfc64simd against scalar sfc64 streams.
#include <stdint.h>
#include <stdbool.h>
#include "randompack.h"
#include "randompack_internal.h"
#include "randompack_config.h"
#include "test_util.h"
#include "xCheck.h"

void TestSfc64simd(void) {
  int n = 500;
  int m = 8*n;
  uint64_t state[4] = {
    0x123456789abcdef0ULL,
    0x0f1e2d3c4b5a6978ULL,
    0x8796a5b4c3d2e1f0ULL,
    0
  };
  uint64_t delta = 1ULL << 61;
  uint64_t xor_scalar[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint64_t xor_simd[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  uint64_t *buf;
  TEST_ALLOC(buf, m);
  randompack_rng *rng_simd = randompack_create("sfc64simd");
  check_rng_clean(rng_simd);
  bool ok = randompack_set_state(state, 4, rng_simd);
  check_success(ok, rng_simd);
  for (int i = 0; i < 8; i++) {
    randompack_rng *rng = randompack_create("sfc64");
    check_rng_clean(rng);
    uint64_t scalar_state[4] = {
      state[0],
      state[1],
      state[2],
      (uint64_t)i*delta
    };
    ok = randompack_set_state(scalar_state, 4, rng);
    check_success(ok, rng);
    ok = randompack_uint64(buf, (size_t)n, 0, rng);
    check_success(ok, rng);
    for (int j = 0; j < n; j++) xor_scalar[i] ^= buf[j];
    randompack_free(rng);
  }
  ok = randompack_uint64(buf, (size_t)m, 0, rng_simd);
  check_success(ok, rng_simd);
  for (int i = 0; i < m; i++) xor_simd[i & 7] ^= buf[i];
  for (int i = 0; i < 8; i++) xCheck(xor_scalar[i] == xor_simd[i]);
  randompack_free(rng_simd);
  FREE(buf);
}
