// Checking x256++simd against x256++, and x256**simd against x256**.
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
  char *scalar_engines[] = {"x256++", "x256**"};
  char *simd_engines[] = {"x256++simd", "x256**simd"};
  int n = 10000;
  int m = 8*n;
  uint64_t *a, *b;
  TEST_ALLOC(a, n);
  TEST_ALLOC(b, m);
  for (int k = 0; k < LEN(scalar_engines); k++) {
    char *scalar_engine = scalar_engines[k];
    char *simd_engine = simd_engines[k];
    randompack_rng *rng_scalar = create_seeded_rng(scalar_engine, 42);
    randompack_rng *rng_simd = create_seeded_rng(simd_engine, 42);
    check_rng_clean(rng_scalar);
    check_rng_clean(rng_simd);
    bool ok = randompack_uint64(a, (size_t)n, 0, rng_scalar);
    check_success(ok, rng_scalar);
    ok = randompack_uint64(b, (size_t)m, 0, rng_simd);
    check_success(ok, rng_simd);
    uint64_t xor_scalar = 0;
    uint64_t xor_simd0 = 0;
    for (int i = 0; i < n; i++) xor_scalar ^= a[i];
    for (int i = 0; i < m; i += 8) xor_simd0 ^= b[i];
    xCheck(xor_scalar == xor_simd0);
    xCheck(a[n - 1] == b[m - 8]);
    randompack_free(rng_scalar);
    randompack_free(rng_simd);
    randompack_rng *rng_simd2 = create_seeded_rng(simd_engine, 42);
    check_rng_clean(rng_simd2);
    uint64_t states[8][4];
    for (int i = 0; i < 8; i++) get_stream_state(states[i], i, rng_simd2);
    xor_scalar = 0;
    for (int i = 0; i < 8; i++) {
      randompack_rng *r = randompack_create(scalar_engine);
      check_rng_clean(r);
      ok = randompack_set_state(states[i], 4, r);
      check_success(ok, r);
      ok = randompack_uint64(b, (size_t)n, 0, r);
      check_success(ok, r);
      for (int j = 0; j < n; j++) xor_scalar ^= b[j];
      randompack_free(r);
    }
    ok = randompack_uint64(b, (size_t)m, 0, rng_simd2);
    check_success(ok, rng_simd2);
    uint64_t xor_simd = 0;
    for (int i = 0; i < m; i++) xor_simd ^= b[i];
    xCheck(xor_scalar == xor_simd);
    randompack_free(rng_simd2);
  }
  FREE(b);
  FREE(a);
}
