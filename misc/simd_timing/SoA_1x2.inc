// -*- C -*-
#if HAVE_NEON
#include <arm_neon.h>
#define VEC_T uint64x2_t
#define VEC_LOAD(p) vld1q_u64((const uint64_t *)(p))
#define VEC_STORE(p, x) vst1q_u64((uint64_t *)(p), (x))
#define VEC_ADD(a, b) vaddq_u64((a), (b))
#define VEC_XOR(a, b) veorq_u64((a), (b))
#define VEC_SHL17(a) vshlq_n_u64((a), 17)
#define VEC_ROTL23(x) vorrq_u64(vshlq_n_u64((x), 23), vshrq_n_u64((x), 41))
#define VEC_ROTL45(x) vorrq_u64(vshlq_n_u64((x), 45), vshrq_n_u64((x), 19))
#define HAVE_VEC 1
#elif HAVE_AVX2
#include <immintrin.h>
#define VEC_T __m128i
#define VEC_LOAD(p) _mm_loadu_si128((const __m128i *)(p))
#define VEC_STORE(p, x) _mm_storeu_si128((__m128i *)(p), (x))
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
  xo256 *st = &rng->state.xo;
  VEC_T s0 = VEC_LOAD(&st->s0[0]);
  VEC_T s1 = VEC_LOAD(&st->s1[0]);
  VEC_T s2 = VEC_LOAD(&st->s2[0]);
  VEC_T s3 = VEC_LOAD(&st->s3[0]);
  for (size_t i = 0; i < len; i += 2) {
    VEC_T r = VEC_ADD(VEC_ROTL23(VEC_ADD(s0, s3)), s0);
    VEC_T t = VEC_SHL17(s1);
    s2 = VEC_XOR(s2, s0);
    s3 = VEC_XOR(s3, s1);
    s1 = VEC_XOR(s1, s2);
    s0 = VEC_XOR(s0, s3);
    s2 = VEC_XOR(s2, t);
    s3 = VEC_ROTL45(s3);
    VEC_STORE(out + i, r);
  }
  VEC_STORE(&st->s0[0], s0);
  VEC_STORE(&st->s1[0], s1);
  VEC_STORE(&st->s2[0], s2);
  VEC_STORE(&st->s3[0], s3);
}
#else
static inline void fill_fast(randompack_rng *rng, size_t len) { // 2 by 1
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  xo256 *st = &rng->state.xo;
  size_t n = len;
  uint64_t a0 = st->s0[0], a1 = st->s1[0], a2 = st->s2[0], a3 = st->s3[0];
  uint64_t b0 = st->s0[1], b1 = st->s1[1], b2 = st->s2[1], b3 = st->s3[1];
  uint64_t ra, rb, ta, tb;
  for (size_t i = 0; i < n; i += 2) {
    FAST_STEP_SCALAR(a0, a1, a2, a3, out[i]);
    FAST_STEP_SCALAR(b0, b1, b2, b3, out[i+1]);
  }
  st->s0[0] = a0; st->s1[0] = a1; st->s2[0] = a2; st->s3[0] = a3;
  st->s0[1] = b0; st->s1[1] = b1; st->s2[1] = b2; st->s3[1] = b3;
}
#endif
