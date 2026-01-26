// -*- C -*-
static inline void fill_fast(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  size_t n = len;
  uint64_t a0 = rng->state.ustream[0][0];
  uint64_t a1 = rng->state.ustream[0][1];
  uint64_t a2 = rng->state.ustream[0][2];
  uint64_t a3 = rng->state.ustream[0][3];
  uint64_t b0 = rng->state.ustream[1][0];
  uint64_t b1 = rng->state.ustream[1][1];
  uint64_t b2 = rng->state.ustream[1][2];
  uint64_t b3 = rng->state.ustream[1][3];
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
    out[i+1] = rb;
  }
  rng->state.ustream[0][0] = a0;
  rng->state.ustream[0][1] = a1;
  rng->state.ustream[0][2] = a2;
  rng->state.ustream[0][3] = a3;
  rng->state.ustream[1][0] = b0;
  rng->state.ustream[1][1] = b1;
  rng->state.ustream[1][2] = b2;
  rng->state.ustream[1][3] = b3;
}
