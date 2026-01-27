// -*- C -*-
#define FAST_STEP_SCALAR(s0,s1,s2,s3,outv) do { \
  uint64_t r_; \
  uint64_t t_; \
  r_ = ROTL((s0) + (s3), 23) + (s0); \
  t_ = (s1) << 17; \
  (s2) ^= (s0); \
  (s3) ^= (s1); \
  (s1) ^= (s2); \
  (s0) ^= (s3); \
  (s2) ^= t_; \
  (s3) = ROTL((s3), 45); \
  (outv) = r_; \
 } while (0)

static inline void fill_fast(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  xo256 *st = &rng->state.xo;

#if HAVE_VEC
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
    FAST_STEP_VEC(a0,a1,a2,a3, ra);
    FAST_STEP_VEC(b0,b1,b2,b3, rb);
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
    FAST_STEP_SCALAR(a0,a1,a2,a3,out[i]);
    FAST_STEP_SCALAR(b0,b1,b2,b3,out[i+1]);
    FAST_STEP_SCALAR(c0,c1,c2,c3,out[i+2]);
    FAST_STEP_SCALAR(d0,d1,d2,d3,out[i+3]);
  }

  st->s0[0]=a0; st->s1[0]=a1; st->s2[0]=a2; st->s3[0]=a3;
  st->s0[1]=b0; st->s1[1]=b1; st->s2[1]=b2; st->s3[1]=b3;
  st->s0[2]=c0; st->s1[2]=c1; st->s2[2]=c2; st->s3[2]=c3;
  st->s0[3]=d0; st->s1[3]=d1; st->s2[3]=d2; st->s3[3]=d3;
#endif
}
