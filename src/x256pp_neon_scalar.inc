// -*- C -*-
#define FAST_STEP_SCALAR_SS(s0,s1,s2,s3,out) do { \
  uint64_t r_ = rotl_u64((s1)*5, 7)*9; \
  uint64_t t_ = (s1) << 17; \
  (s2) ^= (s0); \
  (s3) ^= (s1); \
  (s1) ^= (s2); \
  (s0) ^= (s3); \
  (s2) ^= t_; \
  (s3) = rotl_u64((s3), 45); \
  (out) = r_; \
} while (0)

#if HAVE_NEON
#include <arm_neon.h>
#define VEC_T uint64x2_t
#define VEC_LOAD(p) vld1q_u64((const uint64_t *)(p))
#define VEC_STORE(p,x) vst1q_u64((uint64_t *)(p),(x))
#define VEC_ADD(a,b) vaddq_u64((a),(b))
#define VEC_XOR(a,b) veorq_u64((a),(b))
#define VEC_SHL17(a) vshlq_n_u64((a),17)
#define VEC_ROTL7(x) vorrq_u64(vshlq_n_u64((x),7),vshrq_n_u64((x),64-7))
#define VEC_ROTL45(x) vorrq_u64(vshlq_n_u64((x),45),vshrq_n_u64((x),64-45))
#define VEC_MUL5(x) VEC_ADD(vshlq_n_u64((x),2),(x))
#define VEC_MUL9(x) VEC_ADD(vshlq_n_u64((x),3),(x))
#define HAVE_VEC 1
#else
#define HAVE_VEC 0
#endif

#if HAVE_VEC
#define FAST_STEP_VEC(s0,s1,s2,s3,outv) do { \
  VEC_T r_; \
  VEC_T t_; \
  r_ = VEC_MUL9(VEC_ROTL7(VEC_MUL5(s1))); \
  t_ = VEC_SHL17((s1)); \
  (s2) = VEC_XOR((s2),(s0)); \
  (s3) = VEC_XOR((s3),(s1)); \
  (s1) = VEC_XOR((s1),(s2)); \
  (s0) = VEC_XOR((s0),(s3)); \
  (s2) = VEC_XOR((s2),t_); \
  (s3) = VEC_ROTL45((s3)); \
  (outv) = r_; \
} while (0)
#endif

static inline void fill_fast(uint64_t *buf, size_t len, randompack_state *state) {
  uint64_t *out = buf;
  xo256 *st = &state->xo;
#if HAVE_NEON
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
#else
  uint64_t a0 = st->s0[0], a1 = st->s1[0], a2 = st->s2[0], a3 = st->s3[0];
  uint64_t b0 = st->s0[1], b1 = st->s1[1], b2 = st->s2[1], b3 = st->s3[1];
  uint64_t c0 = st->s0[2], c1 = st->s1[2], c2 = st->s2[2], c3 = st->s3[2];
  uint64_t d0 = st->s0[3], d1 = st->s1[3], d2 = st->s2[3], d3 = st->s3[3];
  for (size_t i = 0; i < len; i += 4) {
    FAST_STEP_SCALAR_SS(a0,a1,a2,a3,out[i]);
    FAST_STEP_SCALAR_SS(b0,b1,b2,b3,out[i+1]);
    FAST_STEP_SCALAR_SS(c0,c1,c2,c3,out[i+2]);
    FAST_STEP_SCALAR_SS(d0,d1,d2,d3,out[i+3]);
  }
  st->s0[0]=a0; st->s1[0]=a1; st->s2[0]=a2; st->s3[0]=a3;
  st->s0[1]=b0; st->s1[1]=b1; st->s2[1]=b2; st->s3[1]=b3;
  st->s0[2]=c0; st->s1[2]=c1; st->s2[2]=c2; st->s3[2]=c3;
  st->s0[3]=d0; st->s1[3]=d1; st->s2[3]=d2; st->s3[3]=d3;
#endif
}
