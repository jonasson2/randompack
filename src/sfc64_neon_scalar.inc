// -*- C -*-

#if HAVE_VEC
#define SFC64_STEP_VEC(a, b, c, ctr, one, outv) do { \
  VEC_T t_; \
  (outv) = VEC_ADD(VEC_ADD((a), (b)), (ctr)); \
  (ctr) = VEC_ADD((ctr), (one)); \
  t_ = VEC_XOR((b), VEC_SHR11((b))); \
  (b) = VEC_ADD((c), VEC_SHL3((c))); \
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
  for (; i + 16 <= len; i += 16) {
    VEC_T r0, r1, s0, s1, t0, t1, u0, u1;
    SFC64_STEP_VEC(a0, b0, c0, d0, one, r0);
    SFC64_STEP_VEC(a1, b1, c1, d1, one, r1);
    VEC_STORE(out + i, r0);
    VEC_STORE(out + i + 2, r1);
    SFC64_STEP_VEC(a0, b0, c0, d0, one, s0);
    SFC64_STEP_VEC(a1, b1, c1, d1, one, s1);
    VEC_STORE(out + i + 4, s0);
    VEC_STORE(out + i + 6, s1);
    SFC64_STEP_VEC(a0, b0, c0, d0, one, t0);
    SFC64_STEP_VEC(a1, b1, c1, d1, one, t1);
    VEC_STORE(out + i + 8, t0);
    VEC_STORE(out + i + 10, t1);
    SFC64_STEP_VEC(a0, b0, c0, d0, one, u0);
    SFC64_STEP_VEC(a1, b1, c1, d1, one, u1);
    VEC_STORE(out + i + 12, u0);
    VEC_STORE(out + i + 14, u1);
  }
  for (; i + 8 <= len; i += 8) {
    VEC_T r0, r1, s0, s1;
    SFC64_STEP_VEC(a0, b0, c0, d0, one, r0);
    SFC64_STEP_VEC(a1, b1, c1, d1, one, r1);
    VEC_STORE(out + i, r0);
    VEC_STORE(out + i + 2, r1);
    SFC64_STEP_VEC(a0, b0, c0, d0, one, s0);
    SFC64_STEP_VEC(a1, b1, c1, d1, one, s1);
    VEC_STORE(out + i + 4, s0);
    VEC_STORE(out + i + 6, s1);
  }
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
  uint64_t a0 = st->s0[0], b0 = st->s1[0], c0 = st->s2[0], d0 = st->s3[0];
  uint64_t a1 = st->s0[1], b1 = st->s1[1], c1 = st->s2[1], d1 = st->s3[1];
  uint64_t a2 = st->s0[2], b2 = st->s1[2], c2 = st->s2[2], d2 = st->s3[2];
  uint64_t a3 = st->s0[3], b3 = st->s1[3], c3 = st->s2[3], d3 = st->s3[3];
  for (size_t i = 0; i < len; i += 4) {
    uint64_t r0 = a0 + b0 + d0++;
    uint64_t r1 = a1 + b1 + d1++;
    uint64_t r2 = a2 + b2 + d2++;
    uint64_t r3 = a3 + b3 + d3++;
    uint64_t na0 = b0 ^ (b0 >> 11);
    uint64_t na1 = b1 ^ (b1 >> 11);
    uint64_t na2 = b2 ^ (b2 >> 11);
    uint64_t na3 = b3 ^ (b3 >> 11);
    b0 = c0 + (c0 << 3);
    b1 = c1 + (c1 << 3);
    b2 = c2 + (c2 << 3);
    b3 = c3 + (c3 << 3);
    c0 = ROTL(c0, 24) + r0;
    c1 = ROTL(c1, 24) + r1;
    c2 = ROTL(c2, 24) + r2;
    c3 = ROTL(c3, 24) + r3;
    a0 = na0;
    a1 = na1;
    a2 = na2;
    a3 = na3;
    out[i] = r0;
    out[i + 1] = r1;
    out[i + 2] = r2;
    out[i + 3] = r3;
  }
  st->s0[0] = a0; st->s1[0] = b0; st->s2[0] = c0; st->s3[0] = d0;
  st->s0[1] = a1; st->s1[1] = b1; st->s2[1] = c1; st->s3[1] = d1;
  st->s0[2] = a2; st->s1[2] = b2; st->s2[2] = c2; st->s3[2] = d2;
  st->s0[3] = a3; st->s1[3] = b3; st->s2[3] = c3; st->s3[3] = d3;
#endif
}
