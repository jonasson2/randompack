// Checking x256++simd against x256++.
#include <stdint.h>
#include <stdbool.h>
#include "randompack.h"
#include "randompack_internal.h"
#include "randompack_config.h"
#include "test_util.h"
#include "xCheck.h"

static inline void get_stream_state(uint64_t stream_state[4], int i,
  randompack_rng *rng) {
  xo256 *st = &rng->state.xo;
  stream_state[0] = st->s0[i];
  stream_state[1] = st->s1[i];
  stream_state[2] = st->s2[i];
  stream_state[3] = st->s3[i];
}

void TestX256ppsimd(void) {
  int n = 10000;
  int m = 8*n;
  uint64_t *a, *b;
  TEST_ALLOC(a, n);
  TEST_ALLOC(b, m);
  randompack_rng *rng_pp = create_seeded_rng("x256++", 42);
  randompack_rng *rng_fast = create_seeded_rng("x256++simd", 42);
  check_rng_clean(rng_pp);
  check_rng_clean(rng_fast);
  bool ok = randompack_uint64(a, (size_t)n, 0, rng_pp);
  check_success(ok, rng_pp);
  ok = randompack_uint64(b, (size_t)m, 0, rng_fast);
  check_success(ok, rng_fast);
  uint64_t xora = 0, xorb = 0;
  for (int i = 0; i < n; i++) xora ^= a[i];
  for (int i = 0; i < m; i += 8) xorb ^= b[i];
  xCheck(xora == xorb);
  xCheck(a[n - 1] == b[m - 8]);
  randompack_free(rng_pp);
  randompack_free(rng_fast);
  randompack_rng *rng_fast2 = create_seeded_rng("x256++simd", 42);
  check_rng_clean(rng_fast2);
  uint64_t states[8][4];
  for (int i = 0; i < 8; i++) get_stream_state(states[i], i, rng_fast2);
  uint64_t xor_pp = 0;
  for (int i = 0; i < 8; i++) {
    randompack_rng *r = randompack_create("x256++");
    check_rng_clean(r);
    ok = randompack_set_state(states[i], 4, r);
    check_success(ok, r);
    ok = randompack_uint64(b, (size_t)n, 0, r);
    check_success(ok, r);
    for (int j = 0; j < n; j++) xor_pp ^= b[j];
    randompack_free(r);
  }
  ok = randompack_uint64(b, (size_t)m, 0, rng_fast2);
  check_success(ok, rng_fast2);
  uint64_t xor_fast = 0;
  for (int i = 0; i < m; i++) xor_fast ^= b[i];
  xCheck(xor_pp == xor_fast);
  randompack_free(rng_fast2);
  FREE(b);
  FREE(a);
}
