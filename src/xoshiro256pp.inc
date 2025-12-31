// -*- C -*-
// The core of this file is copied from prng.di.unimi.it/xoshiro256plusplus.c, written in
// 2019 by David Blackman and Sebastiano Vigna (vigna@acm.org; public domain).

static inline void fill_x256pp(randompack_rng *rng) {
  uint64_t *out = rng->buf;
  size_t n = BUFSIZE;
  uint64_t s0 = rng->state.u64[0];
  uint64_t s1 = rng->state.u64[1];
  uint64_t s2 = rng->state.u64[2];
  uint64_t s3 = rng->state.u64[3];
  uint64_t result, t;
  for (size_t i = 0; i < n; i++) {
    result = ROTL64(s0 + s3, 23) + s0;
    t = s1 << 17;
    s2 ^= s0;
    s3 ^= s1;
    s1 ^= s2;
    s0 ^= s3;
    s2 ^= t;
    s3 = ROTL64(s3, 45);
    out[i] = result;
  }
  rng->state.u64[0] = s0;
  rng->state.u64[1] = s1;
  rng->state.u64[2] = s2;
  rng->state.u64[3] = s3;
}
