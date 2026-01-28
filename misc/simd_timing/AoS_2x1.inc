// -*- C -*-
static inline void fill_fast(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  size_t n = len;
  uint64_t a0 = rng->state.ustream[0][0];
  uint64_t a1 = rng->state.ustream[0][1];
  uint64_t a2 = rng->state.ustream[0][2];
  uint64_t a3 = rng->state.ustream[0][3];
  uint64_t b0 = rng->state.ustream[1][0];
  uint64_t b1 = rng->state.ustream[1][1];
  uint64_t b2 = rng->state.ustream[1][2];
  uint64_t b3 = rng->state.ustream[1][3];
  uint64_t ra, rb, ta, tb;
  for (size_t i = 0; i < n; i += 2) {
    FAST_STEP_SCALAR(a0, a1, a2, a3, out[i]);
    FAST_STEP_SCALAR(b0, b1, b2, b3, out[i+1]);
  }
  rng->state.ustream[0][0] = a0;
  rng->state.ustream[0][1] = a1;
  rng->state.ustream[0][2] = a2;
  rng->state.ustream[0][3] = a3;
  rng->state.ustream[1][0] = b0;
  rng->state.ustream[1][1] = b1;
  rng->state.ustream[1][2] = b2;
  rng->state.ustream[1][3] = b3;
}
