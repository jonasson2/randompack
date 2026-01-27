// -*- C -*-
// there is no 4 lane SIMD on NEON
#if HAVE_AVX2
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
  xo256 *st = &rng->state.xo;
  VEC_T s0 = VEC_LOAD(&st->s0[0]);
  VEC_T s1 = VEC_LOAD(&st->s1[0]);
  VEC_T s2 = VEC_LOAD(&st->s2[0]);
  VEC_T s3 = VEC_LOAD(&st->s3[0]);
  for (size_t i = 0; i < len; i += 4) {
    VEC_T r;
    FAST_STEP_VEC(s0, s1, s2, s3, r);
    VEC_STORE(out + i, r);
  }
  VEC_STORE(&st->s0[0], s0);
  VEC_STORE(&st->s1[0], s1);
  VEC_STORE(&st->s2[0], s2);
  VEC_STORE(&st->s3[0], s3);
}

#else
static inline void fill_fast(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  xo256 *st = &rng->state.xo;
  uint64_t a0 = st->s0[0], a1 = st->s1[0], a2 = st->s2[0], a3 = st->s3[0];
  uint64_t b0 = st->s0[1], b1 = st->s1[1], b2 = st->s2[1], b3 = st->s3[1];
  uint64_t c0 = st->s0[2], c1 = st->s1[2], c2 = st->s2[2], c3 = st->s3[2];
  uint64_t d0 = st->s0[3], d1 = st->s1[3], d2 = st->s2[3], d3 = st->s3[3];
  for (size_t i = 0; i < len; i += 4) {
    FAST_STEP_SCALAR(a0,a1,a2,a3,out[i]);
    FAST_STEP_SCALAR(b0,b1,b2,b3,out[i+1]);
    FAST_STEP_SCALAR(c0,c1,c2,c3,out[i+2]);
    FAST_STEP_SCALAR(d0,d1,d2,d3,out[i+3]);
  }
  st->s0[0]=a0; st->s1[0]=a1; st->s2[0]=a2; st->s3[0]=a3;
  st->s0[1]=b0; st->s1[1]=b1; st->s2[1]=b2; st->s3[1]=b3;
  st->s0[2]=c0; st->s1[2]=c1; st->s2[2]=c2; st->s3[2]=c3;
  st->s0[3]=d0; st->s1[3]=d1; st->s2[3]=d2; st->s3[3]=d3;
}
#endif
