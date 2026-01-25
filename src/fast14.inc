// -*- C -*-
#if HAVE_NEON
#include <arm_neon.h>
#define VEC_T uint64x2_t
#define VEC_LOAD(p) vld1q_u64((const uint64_t *)(p))
#define VEC_STORE(p,x) vst1q_u64((uint64_t *)(p),(x))
#define VEC_ADD(a,b) vaddq_u64((a),(b))
#define VEC_XOR(a,b) veorq_u64((a),(b))
#define VEC_SHL17(a) vshlq_n_u64((a),17)
#define VEC_ROTL23(x) vorrq_u64(vshlq_n_u64((x),23),vshrq_n_u64((x),64-23))
#define VEC_ROTL45(x) vorrq_u64(vshlq_n_u64((x),45),vshrq_n_u64((x),64-45))
#define HAVE_VEC 1

#elif HAVE_AVX2
#include <immintrin.h>
#define VEC_T __m256i
#define VEC_LOAD(p) _mm256_loadu_si256((const __m256i *)(p))
#define VEC_STORE(p,x) _mm256_storeu_si256((__m256i *)(p),(x))
#define VEC_ADD(a,b) _mm256_add_epi64((a),(b))
#define VEC_XOR(a,b) _mm256_xor_si256((a),(b))
#define VEC_SHL17(a) _mm256_slli_epi64((a),17)
#define VEC_ROTL23(x) _mm256_or_si256(_mm256_slli_epi64((x),23),_mm256_srli_epi64((x),64-23))
#define VEC_ROTL45(x) _mm256_or_si256(_mm256_slli_epi64((x),45),_mm256_srli_epi64((x),64-45))
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
#if HAVE_NEON
  uint64_t A0a[2], A1a[2], A2a[2], A3a[2];
  uint64_t A0b[2], A1b[2], A2b[2], A3b[2];
  VEC_T a0, a1, a2, a3;
  VEC_T b0, b1, b2, b3;
  A0a[0] = s[0][0]; A0a[1] = s[1][0];
  A1a[0] = s[0][1]; A1a[1] = s[1][1];
  A2a[0] = s[0][2]; A2a[1] = s[1][2];
  A3a[0] = s[0][3]; A3a[1] = s[1][3];
  A0b[0] = s[2][0]; A0b[1] = s[3][0];
  A1b[0] = s[2][1]; A1b[1] = s[3][1];
  A2b[0] = s[2][2]; A2b[1] = s[3][2];
  A3b[0] = s[2][3]; A3b[1] = s[3][3];
  a0 = VEC_LOAD(A0a);
  a1 = VEC_LOAD(A1a);
  a2 = VEC_LOAD(A2a);
  a3 = VEC_LOAD(A3a);
  b0 = VEC_LOAD(A0b);
  b1 = VEC_LOAD(A1b);
  b2 = VEC_LOAD(A2b);
  b3 = VEC_LOAD(A3b);
  for (size_t i = 0; i < len; i += 4) {
    VEC_T ra, rb;
    FAST_STEP_VEC(a0, a1, a2, a3, ra);
    FAST_STEP_VEC(b0, b1, b2, b3, rb);
    VEC_STORE(out + i, ra);
    VEC_STORE(out + i + 2, rb);
  }
  VEC_STORE(A0a, a0);
  VEC_STORE(A1a, a1);
  VEC_STORE(A2a, a2);
  VEC_STORE(A3a, a3);
  VEC_STORE(A0b, b0);
  VEC_STORE(A1b, b1);
  VEC_STORE(A2b, b2);
  VEC_STORE(A3b, b3);
  s[0][0] = A0a[0]; s[1][0] = A0a[1];
  s[0][1] = A1a[0]; s[1][1] = A1a[1];
  s[0][2] = A2a[0]; s[1][2] = A2a[1];
  s[0][3] = A3a[0]; s[1][3] = A3a[1];
  s[2][0] = A0b[0]; s[3][0] = A0b[1];
  s[2][1] = A1b[0]; s[3][1] = A1b[1];
  s[2][2] = A2b[0]; s[3][2] = A2b[1];
  s[2][3] = A3b[0]; s[3][3] = A3b[1];
#else
  uint64_t A0[4], A1[4], A2[4], A3[4];
  VEC_T a0, a1, a2, a3;
  A0[0] = s[0][0]; A0[1] = s[1][0]; A0[2] = s[2][0]; A0[3] = s[3][0];
  A1[0] = s[0][1]; A1[1] = s[1][1]; A1[2] = s[2][1]; A1[3] = s[3][1];
  A2[0] = s[0][2]; A2[1] = s[1][2]; A2[2] = s[2][2]; A2[3] = s[3][2];
  A3[0] = s[0][3]; A3[1] = s[1][3]; A3[2] = s[2][3]; A3[3] = s[3][3];
  a0 = VEC_LOAD(A0);
  a1 = VEC_LOAD(A1);
  a2 = VEC_LOAD(A2);
  a3 = VEC_LOAD(A3);
  for (size_t i = 0; i < len; i += 4) {
    VEC_T r;
    FAST_STEP_VEC(a0, a1, a2, a3, r);
    VEC_STORE(out + i, r);
  }
  VEC_STORE(A0, a0);
  VEC_STORE(A1, a1);
  VEC_STORE(A2, a2);
  VEC_STORE(A3, a3);
  s[0][0] = A0[0]; s[1][0] = A0[1]; s[2][0] = A0[2]; s[3][0] = A0[3];
  s[0][1] = A1[0]; s[1][1] = A1[1]; s[2][1] = A1[2]; s[3][1] = A1[3];
  s[0][2] = A2[0]; s[1][2] = A2[1]; s[2][2] = A2[2]; s[3][2] = A2[3];
  s[0][3] = A3[0]; s[1][3] = A3[1]; s[2][3] = A3[2]; s[3][3] = A3[3];
#endif
}

#else // No SIMD
#include "fast41.inc"
#endif
