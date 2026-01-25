// -*- C -*-
#if HAVE_NEON
#include <arm_neon.h>
#define VEC_T uint64x2_t
#define VEC_LOAD(p) vld1q_u64((const uint64_t *)(p))
#define VEC_STORE(p, x) vst1q_u64((uint64_t *)(p), (x))
#define VEC_ADD(a, b) vaddq_u64((a), (b))
#define VEC_XOR(a, b) veorq_u64((a), (b))
#define VEC_SHL17(a) vshlq_n_u64((a), 17)
#define VEC_ROTL23(x) vorrq_u64(vshlq_n_u64((x), 23), vshrq_n_u64((x), 64 - 23))
#define VEC_ROTL45(x) vorrq_u64(vshlq_n_u64((x), 45), vshrq_n_u64((x), 64 - 45))
#define HAVE_VEC 1
#elif HAVE_AVX2
#include <immintrin.h>
#define VEC_T __m128i
#define VEC_LOAD(p) _mm_loadu_si128((const __m128i *)(p))
#define VEC_STORE(p, x) _mm_storeu_si128((__m128i *)(p), (x))
#define VEC_ADD(a, b) _mm_add_epi64((a), (b))
#define VEC_XOR(a, b) _mm_xor_si128((a), (b))
#define VEC_SHL17(a) _mm_slli_epi64((a), 17)
#define VEC_ROTL23(x) _mm_or_si128(_mm_slli_epi64((x), 23), _mm_srli_epi64((x), 64 - 23))
#define VEC_ROTL45(x) _mm_or_si128(_mm_slli_epi64((x), 45), _mm_srli_epi64((x), 64 - 45))
#define HAVE_VEC 1
#else
#define HAVE_VEC 0
#endif

#if HAVE_VEC

static inline void fill_fast(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  size_t n = len;
  uint64_t t0[2], t1[2], t2[2], t3[2];
  t0[0] = rng->state.ustream[0][0]; t0[1] = rng->state.ustream[1][0];
  t1[0] = rng->state.ustream[0][1]; t1[1] = rng->state.ustream[1][1];
  t2[0] = rng->state.ustream[0][2]; t2[1] = rng->state.ustream[1][2];
  t3[0] = rng->state.ustream[0][3]; t3[1] = rng->state.ustream[1][3];
  VEC_T s0 = VEC_LOAD(t0);
  VEC_T s1 = VEC_LOAD(t1);
  VEC_T s2 = VEC_LOAD(t2);
  VEC_T s3 = VEC_LOAD(t3);
  for (size_t i = 0; i < n; i += 2) {
    VEC_T r = VEC_ADD(VEC_ROTL23(VEC_ADD(s0, s3)), s0);
    VEC_T tt = VEC_SHL17(s1);
    s2 = VEC_XOR(s2, s0);
    s3 = VEC_XOR(s3, s1);
    s1 = VEC_XOR(s1, s2);
    s0 = VEC_XOR(s0, s3);
    s2 = VEC_XOR(s2, tt);
    s3 = VEC_ROTL45(s3);
    VEC_STORE(out + i, r);
  }
  VEC_STORE(t0, s0);
  VEC_STORE(t1, s1);
  VEC_STORE(t2, s2);
  VEC_STORE(t3, s3);
  rng->state.ustream[0][0] = t0[0]; rng->state.ustream[1][0] = t0[1];
  rng->state.ustream[0][1] = t1[0]; rng->state.ustream[1][1] = t1[1];
  rng->state.ustream[0][2] = t2[0]; rng->state.ustream[1][2] = t2[1];
  rng->state.ustream[0][3] = t3[0]; rng->state.ustream[1][3] = t3[1];
}

#else
#include "fast11.inc"
#endif
