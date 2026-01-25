// -*- C -*-
#if HAVE_NEON
#include <arm_neon.h>
#define VEC_T uint64x2_t
#define VEC_LOAD(p) vld1q_u64((const uint64_t*)(p))
#define VEC_STORE(p,x) vst1q_u64((uint64_t*)(p),(x))
#define VEC_ADD(a,b) vaddq_u64((a),(b))
#define VEC_XOR(a,b) veorq_u64((a),(b))
#define VEC_SHL17(a) vshlq_n_u64((a),17)
#define VEC_ROTL23(x) vorrq_u64(vshlq_n_u64((x),23),vshrq_n_u64((x),64-23))
#define VEC_ROTL45(x) vorrq_u64(vshlq_n_u64((x),45),vshrq_n_u64((x),64-45))
#define HAVE_VEC 1

#elif HAVE_AVX2
#include <immintrin.h>
#define VEC_T __m128i
#define VEC_LOAD(p) _mm_loadu_si128((const __m128i*)(p))
#define VEC_STORE(p,x) _mm_storeu_si128((__m128i*)(p),(x))
#define VEC_ADD(a,b) _mm_add_epi64((a),(b))
#define VEC_XOR(a,b) _mm_xor_si128((a),(b))
#define VEC_SHL17(a) _mm_slli_epi64((a),17)
#define VEC_ROTL23(x) _mm_or_si128(_mm_slli_epi64((x),23),_mm_srli_epi64((x),64-23))
#define VEC_ROTL45(x) _mm_or_si128(_mm_slli_epi64((x),45),_mm_srli_epi64((x),64-45))
#define HAVE_VEC 1

#else
#define HAVE_VEC 0
#endif

#if HAVE_VEC
#define FAST_STEP_VEC(s0,s1,s2,s3,outv) do { \
  VEC_T r_; \
  VEC_T t_; \
  r_ = VEC_ADD(VEC_ROTL23(VEC_ADD((s0),(s3))),(s0)); \
  t_ = VEC_SHL17((s1)); \
  (s2) = VEC_XOR((s2),(s0)); \
  (s3) = VEC_XOR((s3),(s1)); \
  (s1) = VEC_XOR((s1),(s2)); \
  (s0) = VEC_XOR((s0),(s3)); \
  (s2) = VEC_XOR((s2),t_); \
  (s3) = VEC_ROTL45((s3)); \
  (outv) = r_; \
} while (0)

static inline void fill_fast(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  uint64_t (*s)[4] = rng->state.ustream;
  uint64_t A0[2], A1[2], A2[2], A3[2], B0[2], B1[2], B2[2], B3[2];
  VEC_T a0, a1, a2, a3;
  VEC_T b0, b1, b2, b3;
  A0[0] = s[0][0]; A0[1] = s[1][0];
  A1[0] = s[0][1]; A1[1] = s[1][1];
  A2[0] = s[0][2]; A2[1] = s[1][2];
  A3[0] = s[0][3]; A3[1] = s[1][3];
  B0[0] = s[2][0]; B0[1] = s[3][0];
  B1[0] = s[2][1]; B1[1] = s[3][1];
  B2[0] = s[2][2]; B2[1] = s[3][2];
  B3[0] = s[2][3]; B3[1] = s[3][3];
  a0 = VEC_LOAD(A0);
  a1 = VEC_LOAD(A1);
  a2 = VEC_LOAD(A2);
  a3 = VEC_LOAD(A3);
  b0 = VEC_LOAD(B0);
  b1 = VEC_LOAD(B1);
  b2 = VEC_LOAD(B2);
  b3 = VEC_LOAD(B3);
  for (size_t i = 0; i < len; i += 4) {
    VEC_T ra, rb;
    FAST_STEP_VEC(a0, a1, a2, a3, ra);
    FAST_STEP_VEC(b0, b1, b2, b3, rb);
    VEC_STORE(out + i, ra);
    VEC_STORE(out + i + 2, rb);
  }
  VEC_STORE(A0, a0);
  VEC_STORE(A1, a1);
  VEC_STORE(A2, a2);
  VEC_STORE(A3, a3);
  VEC_STORE(B0, b0);
  VEC_STORE(B1, b1);
  VEC_STORE(B2, b2);
  VEC_STORE(B3, b3);
  s[0][0] = A0[0]; s[1][0] = A0[1];
  s[0][1] = A1[0]; s[1][1] = A1[1];
  s[0][2] = A2[0]; s[1][2] = A2[1];
  s[0][3] = A3[0]; s[1][3] = A3[1];
  s[2][0] = B0[0]; s[3][0] = B0[1];
  s[2][1] = B1[0]; s[3][1] = B1[1];
  s[2][2] = B2[0]; s[3][2] = B2[1];
  s[2][3] = B3[0]; s[3][3] = B3[1];
}

#else // No SIMD
#include "fast41.inc"
#endif
