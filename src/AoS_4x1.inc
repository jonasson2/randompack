// -*- C -*-
static inline void fill_fast(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  uint64_t (*s)[4] = rng->state.ustream;
  uint64_t a0, a1, a2, a3;
  uint64_t b0, b1, b2, b3;
  uint64_t c0, c1, c2, c3;
  uint64_t d0, d1, d2, d3;
  a0 = s[0][0]; a1 = s[0][1]; a2 = s[0][2]; a3 = s[0][3];
  b0 = s[1][0]; b1 = s[1][1]; b2 = s[1][2]; b3 = s[1][3];
  c0 = s[2][0]; c1 = s[2][1]; c2 = s[2][2]; c3 = s[2][3];
  d0 = s[3][0]; d1 = s[3][1]; d2 = s[3][2]; d3 = s[3][3];
  for (size_t i = 0; i < len; i += 4) {
    FAST_STEP_SCALAR(a0, a1, a2, a3, out[i]);
    FAST_STEP_SCALAR(b0, b1, b2, b3, out[i + 1]);
    FAST_STEP_SCALAR(c0, c1, c2, c3, out[i + 2]);
    FAST_STEP_SCALAR(d0, d1, d2, d3, out[i + 3]);
  }
  s[0][0] = a0; s[0][1] = a1; s[0][2] = a2; s[0][3] = a3;
  s[1][0] = b0; s[1][1] = b1; s[1][2] = b2; s[1][3] = b3;
  s[2][0] = c0; s[2][1] = c1; s[2][2] = c2; s[2][3] = c3;
  s[3][0] = d0; s[3][1] = d1; s[3][2] = d2; s[3][3] = d3;
}
