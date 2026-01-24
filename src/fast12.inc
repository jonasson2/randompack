// -*- C -*-
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
#include <arm_neon.h>

static inline uint64x2_t rotl64_23(uint64x2_t x) {
  return vorrq_u64(vshlq_n_u64(x, 23), vshrq_n_u64(x, 64 - 23));
}

static inline uint64x2_t rotl64_45(uint64x2_t x) {
  return vorrq_u64(vshlq_n_u64(x, 45), vshrq_n_u64(x, 64 - 45));
}

static inline void fill_fast(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  size_t n = len;
  uint64_t t0[2], t1[2], t2[2], t3[2];
  t0[0] = rng->state.ustream[0][0]; t0[1] = rng->state.ustream[1][0];
  t1[0] = rng->state.ustream[0][1]; t1[1] = rng->state.ustream[1][1];
  t2[0] = rng->state.ustream[0][2]; t2[1] = rng->state.ustream[1][2];
  t3[0] = rng->state.ustream[0][3]; t3[1] = rng->state.ustream[1][3];
  uint64x2_t s0 = vld1q_u64(t0);
  uint64x2_t s1 = vld1q_u64(t1);
  uint64x2_t s2 = vld1q_u64(t2);
  uint64x2_t s3 = vld1q_u64(t3);

  for (size_t i = 0; i < n; i += 2) {
    uint64x2_t r = vaddq_u64(rotl64_23(vaddq_u64(s0, s3)), s0);
    uint64x2_t tt = vshlq_n_u64(s1, 17);
    s2 = veorq_u64(s2, s0);
    s3 = veorq_u64(s3, s1);
    s1 = veorq_u64(s1, s2);
    s0 = veorq_u64(s0, s3);
    s2 = veorq_u64(s2, tt);
    s3 = rotl64_45(s3);
    vst1q_u64(out + i, r);
  }

  vst1q_u64(t0, s0);
  vst1q_u64(t1, s1);
  vst1q_u64(t2, s2);
  vst1q_u64(t3, s3);
  rng->state.ustream[0][0] = t0[0]; rng->state.ustream[1][0] = t0[1];
  rng->state.ustream[0][1] = t1[0]; rng->state.ustream[1][1] = t1[1];
  rng->state.ustream[0][2] = t2[0]; rng->state.ustream[1][2] = t2[1];
  rng->state.ustream[0][3] = t3[0]; rng->state.ustream[1][3] = t3[1];
}
#endif
