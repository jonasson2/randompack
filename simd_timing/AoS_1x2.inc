// -*- C -*-
#if HAVE_NEON
#include <arm_neon.h>
#define VEC_T uint64x2_t
#define VEC_SET2(lo, hi) vsetq_lane_u64((hi), vsetq_lane_u64((lo), vdupq_n_u64(0), 0), 1)
#define VEC_STOREU64(p, x) vst1q_u64((uint64_t *)(p), (x))
#define VEC_ADD(a, b) vaddq_u64((a), (b))
#define VEC_XOR(a, b) veorq_u64((a), (b))
#define VEC_SHL17(a) vshlq_n_u64((a), 17)
#define VEC_ROTL23(x) vorrq_u64(vshlq_n_u64((x), 23), vshrq_n_u64((x), 41))
#define VEC_ROTL45(x) vorrq_u64(vshlq_n_u64((x), 45), vshrq_n_u64((x), 19))
#define HAVE_VEC 1
#elif HAVE_AVX2
#include <immintrin.h>
#define VEC_T __m128i
#define VEC_SET2(lo, hi) _mm_set_epi64x((long long)(hi), (long long)(lo))
#define VEC_STOREU64(p, x) _mm_storeu_si128((__m128i *)(p), (x))
#define VEC_ADD(a, b) _mm_add_epi64((a), (b))
#define VEC_XOR(a, b) _mm_xor_si128((a), (b))
#define VEC_SHL17(a) _mm_slli_epi64((a), 17)
#define VEC_ROTL23(x) _mm_or_si128(_mm_slli_epi64((x), 23), _mm_srli_epi64((x), 41))
#define VEC_ROTL45(x) _mm_or_si128(_mm_slli_epi64((x), 45), _mm_srli_epi64((x), 19))
#define HAVE_VEC 1
#else
#define HAVE_VEC 0
#endif

#if HAVE_VEC
static inline void fill_fast(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  uint64_t (*s)[4] = rng->state.ustream;
  uint64_t t0[2], t1[2], t2[2], t3[2];
  VEC_T s0 = VEC_SET2(s[0][0], s[1][0]);
  VEC_T s1 = VEC_SET2(s[0][1], s[1][1]);
  VEC_T s2 = VEC_SET2(s[0][2], s[1][2]);
  VEC_T s3 = VEC_SET2(s[0][3], s[1][3]);
  for (size_t i = 0; i < len; i += 2) {
    VEC_T r = VEC_ADD(VEC_ROTL23(VEC_ADD(s0, s3)), s0);
    VEC_T tt = VEC_SHL17(s1);
    s2 = VEC_XOR(s2, s0);
    s3 = VEC_XOR(s3, s1);
    s1 = VEC_XOR(s1, s2);
    s0 = VEC_XOR(s0, s3);
    s2 = VEC_XOR(s2, tt);
    s3 = VEC_ROTL45(s3);
    VEC_STOREU64(out + i, r);
  }
  VEC_STOREU64(t0, s0);
  VEC_STOREU64(t1, s1);
  VEC_STOREU64(t2, s2);
  VEC_STOREU64(t3, s3);
  s[0][0] = t0[0]; s[1][0] = t0[1];
  s[0][1] = t1[0]; s[1][1] = t1[1];
  s[0][2] = t2[0]; s[1][2] = t2[1];
  s[0][3] = t3[0]; s[1][3] = t3[1];
}
#else
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
#endif
