// -*- C -*-
// normref.inc: frozen reference snapshot of norm_exp.inc for timing calibration.
// All functions renamed (*_ref) to coexist with norm_exp.inc in the same TU.
// DO NOT MODIFY — this is the baseline against which optimizations are measured.

NORMEXP_INLINE double flip_sign_if_ref(double x, int sign) {
  uint64_t bits, mask = (uint64_t)0 - (uint64_t)(sign & 1);
  mask &= DBL_SIGNBIT;
  memcpy(&bits, &x, sizeof bits);
  bits ^= mask;
  memcpy(&x, &bits, sizeof bits);
  return x;
}

NORMEXP_INLINE double norm_tail_draw_ref(randompack_rng *rng, uint64_t rabs, double y, int *w) {
  double xx, yy, x;
  for (;;) {
    x = draw_u01(rng, w);
    xx = -ziggurat_nor_inv_r * openlibm_log1p(-x);
    yy = -openlibm_log1p(-y);
    if (LIKELY(yy + yy > xx * xx))
      return ((rabs >> 8) & 0x1) ? -(ziggurat_nor_r + xx) : ziggurat_nor_r + xx;
    y = draw_u01(rng, w);
  }
}

NORMEXP_INLINE double norm_likely_ref(uint64_t r, uint8_t *idx, uint64_t *ki, uint64_t *rabs) {
  int sign;
  double x;
#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64)
  memcpy(idx, (uint8_t*)(&r) + 7, 1);
#else
  *idx = r >> 56;
#endif
  *ki = ki_double[*idx];
  sign = (r >> 52) & 1;
  *rabs = r & 0x000fffffffffffff;
  x = (*rabs)*wi_double[*idx];
  x = flip_sign_if_ref(x, sign);
  return x;
}

NORMEXP_INLINE double norm_unlikely_ref(randompack_rng *rng, int *w, double x, uint8_t idx,
                                        uint64_t *rabs, uint64_t ki) {
  double y, f0, f1, tangent;
  uint64_t r, dr, dk;
  while (true) {
    y = draw_u01(rng, w);
    dr = (1ULL << 52) - *rabs;
    tangent = dr*slope_norm[idx];
    if (LIKELY(idx > 52)) {
      if (y <= tangent) return x;
      dk = (1ULL << 52) - ki;
      if (y*dk > dr) goto next_try;
    }
    else if (UNLIKELY(idx == 0)) {
      x = norm_tail_draw_ref(rng, *rabs, y, w);
      return x;
    }
    else if (LIKELY(idx < 52)) {
      if (y > tangent) goto next_try;
      uint64_t dk = (1ULL << 52) - ki;
      if (y*dk <= dr) return x;
    }
    f0 = fi_double[idx-1];
    f1 = fi_double[idx];
    if (f1 + y*(f0 - f1) < openlibm_exp(-x*x/2)) return x;
  next_try:
    r = draw_u64(rng, w);
    x = norm_likely_ref(r, &idx, &ki, rabs);
    if (*rabs < ki) return x;
  }
}

NORMEXP_INLINE void rand_normref(double *buf, size_t len, randompack_rng *rng) {
  align64(rng);
  draw_raw_copy((uint8_t *)buf, len*sizeof(uint64_t), rng);
  int w = enter_u64_mode(rng), sign;
  uint8_t idx;
  uint64_t r, rabs, ki;
  int64_t srabs;
  double x;
  for (size_t i = 0; i < len; i++) {
    uint8_t *bufpi = (uint8_t*)(buf + i);
    memcpy(&r, bufpi, sizeof(r));
#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64)
    memcpy(&idx, bufpi + 7, 1);
#else
    idx = r >> 56;
#endif
    ki = ki_double[idx];
    sign = (r >> 52) & 1;
    rabs = r & 0x000fffffffffffff;
    srabs = sign ? -(int64_t)rabs : (int64_t)rabs;
    x = srabs*wi_double[idx];
    buf[i] = LIKELY(rabs < ki) ? x : norm_unlikely_ref(rng, &w, x, idx, &rabs, ki);
  }
  exit_u64_mode(rng, w);
}
