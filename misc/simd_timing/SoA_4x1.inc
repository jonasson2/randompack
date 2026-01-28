// -*- C -*-
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
