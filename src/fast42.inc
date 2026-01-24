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
  uint64_t a0_[2], a1_[2], a2_[2], a3_[2];
  uint64_t b0_[2], b1_[2], b2_[2], b3_[2];
  uint64_t c0_[2], c1_[2], c2_[2], c3_[2];
  uint64_t d0_[2], d1_[2], d2_[2], d3_[2];
  a0_[0] = rng->state.ustream[0][0]; a0_[1] = rng->state.ustream[1][0];
  a1_[0] = rng->state.ustream[0][1]; a1_[1] = rng->state.ustream[1][1];
  a2_[0] = rng->state.ustream[0][2]; a2_[1] = rng->state.ustream[1][2];
  a3_[0] = rng->state.ustream[0][3]; a3_[1] = rng->state.ustream[1][3];
  b0_[0] = rng->state.ustream[2][0]; b0_[1] = rng->state.ustream[3][0];
  b1_[0] = rng->state.ustream[2][1]; b1_[1] = rng->state.ustream[3][1];
  b2_[0] = rng->state.ustream[2][2]; b2_[1] = rng->state.ustream[3][2];
  b3_[0] = rng->state.ustream[2][3]; b3_[1] = rng->state.ustream[3][3];
  c0_[0] = rng->state.ustream[4][0]; c0_[1] = rng->state.ustream[5][0];
  c1_[0] = rng->state.ustream[4][1]; c1_[1] = rng->state.ustream[5][1];
  c2_[0] = rng->state.ustream[4][2]; c2_[1] = rng->state.ustream[5][2];
  c3_[0] = rng->state.ustream[4][3]; c3_[1] = rng->state.ustream[5][3];
  d0_[0] = rng->state.ustream[6][0]; d0_[1] = rng->state.ustream[7][0];
  d1_[0] = rng->state.ustream[6][1]; d1_[1] = rng->state.ustream[7][1];
  d2_[0] = rng->state.ustream[6][2]; d2_[1] = rng->state.ustream[7][2];
  d3_[0] = rng->state.ustream[6][3]; d3_[1] = rng->state.ustream[7][3];
  uint64x2_t a0 = vld1q_u64(a0_);
  uint64x2_t a1 = vld1q_u64(a1_);
  uint64x2_t a2 = vld1q_u64(a2_);
  uint64x2_t a3 = vld1q_u64(a3_);
  uint64x2_t b0 = vld1q_u64(b0_);
  uint64x2_t b1 = vld1q_u64(b1_);
  uint64x2_t b2 = vld1q_u64(b2_);
  uint64x2_t b3 = vld1q_u64(b3_);
  uint64x2_t c0 = vld1q_u64(c0_);
  uint64x2_t c1 = vld1q_u64(c1_);
  uint64x2_t c2 = vld1q_u64(c2_);
  uint64x2_t c3 = vld1q_u64(c3_);
  uint64x2_t d0 = vld1q_u64(d0_);
  uint64x2_t d1 = vld1q_u64(d1_);
  uint64x2_t d2 = vld1q_u64(d2_);
  uint64x2_t d3 = vld1q_u64(d3_);
  for (size_t i = 0; i < n; i += 8) {
    uint64x2_t ra = vaddq_u64(rotl64_23(vaddq_u64(a0, a3)), a0);
    uint64x2_t ta = vshlq_n_u64(a1, 17);
    a2 = veorq_u64(a2, a0);
    a3 = veorq_u64(a3, a1);
    a1 = veorq_u64(a1, a2);
    a0 = veorq_u64(a0, a3);
    a2 = veorq_u64(a2, ta);
    a3 = rotl64_45(a3);
    uint64x2_t rb = vaddq_u64(rotl64_23(vaddq_u64(b0, b3)), b0);
    uint64x2_t tb = vshlq_n_u64(b1, 17);
    b2 = veorq_u64(b2, b0);
    b3 = veorq_u64(b3, b1);
    b1 = veorq_u64(b1, b2);
    b0 = veorq_u64(b0, b3);
    b2 = veorq_u64(b2, tb);
    b3 = rotl64_45(b3);
    uint64x2_t rc = vaddq_u64(rotl64_23(vaddq_u64(c0, c3)), c0);
    uint64x2_t tc = vshlq_n_u64(c1, 17);
    c2 = veorq_u64(c2, c0);
    c3 = veorq_u64(c3, c1);
    c1 = veorq_u64(c1, c2);
    c0 = veorq_u64(c0, c3);
    c2 = veorq_u64(c2, tc);
    c3 = rotl64_45(c3);
    uint64x2_t rd = vaddq_u64(rotl64_23(vaddq_u64(d0, d3)), d0);
    uint64x2_t td = vshlq_n_u64(d1, 17);
    d2 = veorq_u64(d2, d0);
    d3 = veorq_u64(d3, d1);
    d1 = veorq_u64(d1, d2);
    d0 = veorq_u64(d0, d3);
    d2 = veorq_u64(d2, td);
    d3 = rotl64_45(d3);
    vst1q_u64(out + i, ra);
    vst1q_u64(out + i + 2, rb);
    vst1q_u64(out + i + 4, rc);
    vst1q_u64(out + i + 6, rd);
  }
  vst1q_u64(a0_, a0);
  vst1q_u64(a1_, a1);
  vst1q_u64(a2_, a2);
  vst1q_u64(a3_, a3);
  vst1q_u64(b0_, b0);
  vst1q_u64(b1_, b1);
  vst1q_u64(b2_, b2);
  vst1q_u64(b3_, b3);
  vst1q_u64(c0_, c0);
  vst1q_u64(c1_, c1);
  vst1q_u64(c2_, c2);
  vst1q_u64(c3_, c3);
  vst1q_u64(d0_, d0);
  vst1q_u64(d1_, d1);
  vst1q_u64(d2_, d2);
  vst1q_u64(d3_, d3);
  rng->state.ustream[0][0] = a0_[0]; rng->state.ustream[1][0] = a0_[1];
  rng->state.ustream[0][1] = a1_[0]; rng->state.ustream[1][1] = a1_[1];
  rng->state.ustream[0][2] = a2_[0]; rng->state.ustream[1][2] = a2_[1];
  rng->state.ustream[0][3] = a3_[0]; rng->state.ustream[1][3] = a3_[1];
  rng->state.ustream[2][0] = b0_[0]; rng->state.ustream[3][0] = b0_[1];
  rng->state.ustream[2][1] = b1_[0]; rng->state.ustream[3][1] = b1_[1];
  rng->state.ustream[2][2] = b2_[0]; rng->state.ustream[3][2] = b2_[1];
  rng->state.ustream[2][3] = b3_[0]; rng->state.ustream[3][3] = b3_[1];
  rng->state.ustream[4][0] = c0_[0]; rng->state.ustream[5][0] = c0_[1];
  rng->state.ustream[4][1] = c1_[0]; rng->state.ustream[5][1] = c1_[1];
  rng->state.ustream[4][2] = c2_[0]; rng->state.ustream[5][2] = c2_[1];
  rng->state.ustream[4][3] = c3_[0]; rng->state.ustream[5][3] = c3_[1];
  rng->state.ustream[6][0] = d0_[0]; rng->state.ustream[7][0] = d0_[1];
  rng->state.ustream[6][1] = d1_[0]; rng->state.ustream[7][1] = d1_[1];
  rng->state.ustream[6][2] = d2_[0]; rng->state.ustream[7][2] = d2_[1];
  rng->state.ustream[6][3] = d3_[0]; rng->state.ustream[7][3] = d3_[1];
}
#else

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
#endif
