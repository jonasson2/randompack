// -*- C -*-

#if HAVE_VEC
#define SFC64_STEP_VEC(a, b, c, ctr, one, outv) do { \
  VEC_T t_; \
  (outv) = VEC_ADD(VEC_ADD((a), (b)), (ctr)); \
  (ctr) = VEC_ADD((ctr), (one)); \
  t_ = VEC_XOR((b), VEC_SHR11((b))); \
  (b) = VEC_MUL9((c)); \
  (c) = VEC_ADD(VEC_ROTL24((c)), (outv)); \
  (a) = t_; \
} while (0)
#endif

static inline void fill_sfc64simd(uint64_t *buf, size_t len, randompack_state *state) {
  uint64_t *out = buf;
  xo256 *st = &state->xo;
#if HAVE_NEON
  VEC_T a0 = VEC_LOAD(&st->s0[0]);
  VEC_T b0 = VEC_LOAD(&st->s1[0]);
  VEC_T c0 = VEC_LOAD(&st->s2[0]);
  VEC_T d0 = VEC_LOAD(&st->s3[0]);
  VEC_T a1 = VEC_LOAD(&st->s0[2]);
  VEC_T b1 = VEC_LOAD(&st->s1[2]);
  VEC_T c1 = VEC_LOAD(&st->s2[2]);
  VEC_T d1 = VEC_LOAD(&st->s3[2]);
  VEC_T one = VEC_SET1(1);
  size_t i = 0;
  for (; i < len; i += 4) {
    VEC_T r0, r1;
    SFC64_STEP_VEC(a0, b0, c0, d0, one, r0);
    SFC64_STEP_VEC(a1, b1, c1, d1, one, r1);
    VEC_STORE(out + i, r0);
    VEC_STORE(out + i + 2, r1);
  }
  VEC_STORE(&st->s0[0], a0);
  VEC_STORE(&st->s1[0], b0);
  VEC_STORE(&st->s2[0], c0);
  VEC_STORE(&st->s3[0], d0);
  VEC_STORE(&st->s0[2], a1);
  VEC_STORE(&st->s1[2], b1);
  VEC_STORE(&st->s2[2], c1);
  VEC_STORE(&st->s3[2], d1);
#else
  for (size_t i = 0; i < len; i += 4) {
    for (int j = 0; j < 4; j++) {
      uint64_t a = st->s0[j];
      uint64_t b = st->s1[j];
      uint64_t c = st->s2[j];
      uint64_t d = st->s3[j];
      out[i + j] = a + b + d;
      st->s3[j] = d + 1;
      st->s0[j] = b ^ (b >> 11);
      st->s1[j] = c + (c << 3);
      st->s2[j] = ROTL(c, 24) + out[i + j];
    }
  }
#endif
}
