// -*- C -*-
static inline void fill_fast(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  size_t n = len;
  uint64_t s0[8], s1[8], s2[8], s3[8];
  for (int j = 0; j < 8; j++) {
    s0[j] = rng->state.ustream[j][0];
    s1[j] = rng->state.ustream[j][1];
    s2[j] = rng->state.ustream[j][2];
    s3[j] = rng->state.ustream[j][3];
  }
  for (size_t i = 0; i < n; i += 8) {
    for (int j = 0; j < 8; j++) {
      uint64_t r = ROTL(s0[j] + s3[j], 23) + s0[j];
      uint64_t t = s1[j] << 17;
      s2[j] ^= s0[j];
      s3[j] ^= s1[j];
      s1[j] ^= s2[j];
      s0[j] ^= s3[j];
      s2[j] ^= t;
      s3[j] = ROTL(s3[j], 45);
      out[i + (size_t)j] = r;
    }
  }
  for (int j = 0; j < 8; j++) {
    rng->state.ustream[j][0] = s0[j];
    rng->state.ustream[j][1] = s1[j];
    rng->state.ustream[j][2] = s2[j];
    rng->state.ustream[j][3] = s3[j];
  }
}

