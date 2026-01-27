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
  xo256 *st = &rng->state.xo;
  VEC_T a0 = VEC_LOAD(&st->s0[0]);
  VEC_T a1 = VEC_LOAD(&st->s1[0]);
  VEC_T a2 = VEC_LOAD(&st->s2[0]);
  VEC_T a3 = VEC_LOAD(&st->s3[0]);
  VEC_T b0 = VEC_LOAD(&st->s0[2]);
  VEC_T b1 = VEC_LOAD(&st->s1[2]);
  VEC_T b2 = VEC_LOAD(&st->s2[2]);
  VEC_T b3 = VEC_LOAD(&st->s3[2]);
  for (size_t i = 0; i < len; i += 4) {
    VEC_T ra, rb;
    FAST_STEP_VEC(a0, a1, a2, a3, ra);
    FAST_STEP_VEC(b0, b1, b2, b3, rb);
    VEC_STORE(out + i, ra);
    VEC_STORE(out + i + 2, rb);
  }
  VEC_STORE(&st->s0[0], a0);
  VEC_STORE(&st->s1[0], a1);
  VEC_STORE(&st->s2[0], a2);
  VEC_STORE(&st->s3[0], a3);
  VEC_STORE(&st->s0[2], b0);
  VEC_STORE(&st->s1[2], b1);
  VEC_STORE(&st->s2[2], b2);
  VEC_STORE(&st->s3[2], b3);
}
#else
#include "fast41soa.inc"
#endif
