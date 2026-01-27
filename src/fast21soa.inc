// -*- C -*-
static inline void fill_fast(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  xo256 *st = &rng->state.xo;
  size_t n = len;
  uint64_t a0 = st->s0[0], a1 = st->s1[0], a2 = st->s2[0], a3 = st->s3[0];
  uint64_t b0 = st->s0[1], b1 = st->s1[1], b2 = st->s2[1], b3 = st->s3[1];
  uint64_t ra, rb, ta, tb;
  for (size_t i = 0; i < n; i += 2) {
    ra = ROTL(a0 + a3, 23) + a0;
    ta = a1 << 17;
    a2 ^= a0;
    a3 ^= a1;
    a1 ^= a2;
    a0 ^= a3;
    a2 ^= ta;
    a3 = ROTL(a3, 45);
    rb = ROTL(b0 + b3, 23) + b0;
    tb = b1 << 17;
    b2 ^= b0;
    b3 ^= b1;
    b1 ^= b2;
    b0 ^= b3;
    b2 ^= tb;
    b3 = ROTL(b3, 45);
    out[i] = ra;
    out[i + 1] = rb;
  }
  st->s0[0] = a0; st->s1[0] = a1; st->s2[0] = a2; st->s3[0] = a3;
  st->s0[1] = b0; st->s1[1] = b1; st->s2[1] = b2; st->s3[1] = b3;
}
