// -*- C -*-
static inline void fill_cwg128(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  // State layout assumed:
  //   rng->state.cwg.x    : __uint128_t
  //   rng->state.cwg.a    : uint64_t
  //   rng->state.cwg.weyl : uint64_t
  //   rng->state.cwg.s    : uint64_t (must be odd)
  __uint128_t x = rng->state.cwg.x;
  uint64_t a = rng->state.cwg.a;
  uint64_t weyl = rng->state.cwg.weyl;
  uint64_t s = rng->state.cwg.s;
  for (size_t i = 0; i < len; i++) {
    a += (uint64_t)x; // modulo 2^64
    weyl += s;
    // x = (x | 1) * ((a += x) >> 1) ^ (weyl += s);
    x = (x | 1) * ((__uint128_t)(a >> 1)) ^ (__uint128_t)weyl;
    // return a >> 48 ^ x;  (take low 64 bits of x for a 64-bit output)
    out[i] = (a >> 48) ^ (uint64_t)x;
  }
  rng->state.cwg.x = x;
  rng->state.cwg.a = a;
  rng->state.cwg.weyl = weyl;
  rng->state.cwg.s = s;
}
