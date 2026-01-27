// -*- C -*-
#define STEP(s0,s1,s2,s3,outv) do { \
  uint64_t r_; \
  uint64_t t_; \
  r_ = ROTL((s0) + (s3), 23) + (s0); \
  t_ = (s1) << 17; \
  (s2) ^= (s0); \
  (s3) ^= (s1); \
  (s1) ^= (s2); \
  (s0) ^= (s3); \
  (s2) ^= t_; \
  (s3) = ROTL((s3), 45); \
  (outv) = r_; \
} while (0)

static inline void fill_fast(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  uint64_t a0 = rng->state.ustream[0][0];
  uint64_t a1 = rng->state.ustream[0][1];
  uint64_t a2 = rng->state.ustream[0][2];
  uint64_t a3 = rng->state.ustream[0][3];
  uint64_t b0 = rng->state.ustream[1][0];
  uint64_t b1 = rng->state.ustream[1][1];
  uint64_t b2 = rng->state.ustream[1][2];
  uint64_t b3 = rng->state.ustream[1][3];
  uint64_t c0 = rng->state.ustream[2][0];
  uint64_t c1 = rng->state.ustream[2][1];
  uint64_t c2 = rng->state.ustream[2][2];
  uint64_t c3 = rng->state.ustream[2][3];
  uint64_t d0 = rng->state.ustream[3][0];
  uint64_t d1 = rng->state.ustream[3][1];
  uint64_t d2 = rng->state.ustream[3][2];
  uint64_t d3 = rng->state.ustream[3][3];
  for (size_t i = 0; i < len; i += 4) {
    STEP(a0,a1,a2,a3,out[i]);
    STEP(b0,b1,b2,b3,out[i + 1]);
    STEP(c0,c1,c2,c3,out[i + 2]);
    STEP(d0,d1,d2,d3,out[i + 3]);
  }
  rng->state.ustream[0][0] = a0;
  rng->state.ustream[0][1] = a1;
  rng->state.ustream[0][2] = a2;
  rng->state.ustream[0][3] = a3;
  rng->state.ustream[1][0] = b0;
  rng->state.ustream[1][1] = b1;
  rng->state.ustream[1][2] = b2;
  rng->state.ustream[1][3] = b3;
  rng->state.ustream[2][0] = c0;
  rng->state.ustream[2][1] = c1;
  rng->state.ustream[2][2] = c2;
  rng->state.ustream[2][3] = c3;
  rng->state.ustream[3][0] = d0;
  rng->state.ustream[3][1] = d1;
  rng->state.ustream[3][2] = d2;
  rng->state.ustream[3][3] = d3;
}
