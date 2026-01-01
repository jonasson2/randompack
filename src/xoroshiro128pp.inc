// -*- C -*-

static inline void fill_xoro128pp(randompack_rng *rng) {
  uint64_t *out = rng->buf;
  uint64_t s0 = rng->state.u64[0];
  uint64_t s1 = rng->state.u64[1];
  for (size_t i = 0; i < BUFSIZE; i++) {
    out[i] = ROTL64(s0 + s1, 17) + s0;
    s1 ^= s0;
    s0 = ROTL64(s0, 49) ^ s1 ^ (s1 << 21);
    s1 = ROTL64(s1, 28);
  }
  rng->state.u64[0] = s0;
  rng->state.u64[1] = s1;
}
