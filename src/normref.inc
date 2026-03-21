// -*- C -*-
// normref.inc: frozen reference snapshot of norm_exp.inc for timing calibration.
// All functions renamed (*_ref) to coexist with norm_exp.inc in the same TU.
// DO NOT MODIFY — this is the baseline against which optimizations are measured.


NEVER_INLINE double norm_tail_draw_ref(randompack_rng *rng, uint64_t rabs, double y, int *w) {
  double xx, yy, x;
  for (;;) {
    /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
    x = draw_u01(rng, w);
    xx = -ziggurat_nor_inv_r * openlibm_log1p(-x);
    yy = -openlibm_log1p(-y);
    if (LIKELY(yy + yy > xx * xx))
      return ((rabs >> 8) & 0x1) ? -(ziggurat_nor_r + xx) : ziggurat_nor_r + xx;
    y = draw_u01(rng, w);
  }
}

NEVER_INLINE bool below_normpdf_ref(uint8_t idx, double x, double y) {
  double f0 = fi_double[idx-1];
  double f1 = fi_double[idx];
  return f1 + y*(f0 - f1) < openlibm_exp(-x*x/2);
}

ALWAYS_INLINE double norm_likely_ref(uint64_t r, uint8_t *idx, uint64_t *ki, uint64_t *rabs) {
  TOP_BYTE(*idx, r);
  *ki = ki_double[*idx];
  int sign = (r >> 52) & 1;
  *rabs = r & 0x000fffffffffffff;
  int64_t srabs = sign ? -(int64_t)*rabs : (int64_t)*rabs;
  return srabs*wi_double[*idx];
}

ALWAYS_INLINE double norm_unlikely_ref(randompack_rng *rng, int *w, double x,
                                       uint8_t idx, uint64_t rabs, uint64_t ki) {
  uint64_t ru, dr, dk;
  int64_t srabs;
  int sign;
  double y, tangent;
  for (;;) {
    y = draw_u01(rng, w);
    dr = (1ULL << 52) - rabs;
    tangent = dr*slope_norm[idx];
    if (LIKELY(idx > 52)) {
      if (y <= tangent) return x;
      dk = (1ULL << 52) - ki;
      if (y*dk > dr) goto next_try;
    }
    else if (UNLIKELY(idx == 0)) {
      x = norm_tail_draw_ref(rng, rabs, y, w);
      return x;
    }
    else if (LIKELY(idx < 52)) {
      if (y > tangent) goto next_try;
      dk = (1ULL << 52) - ki;
      if (y*dk <= dr) return x;
    }
    if (below_normpdf_ref(idx, x, y)) return x;
  next_try:
    ru = draw_u64(rng, w);
    TOP_BYTE(idx, ru);
    ki = ki_double[idx];
    sign = (ru >> 52) & 1;
    rabs = ru & 0x000fffffffffffff;
    srabs = sign ? -(int64_t)rabs : (int64_t)rabs;
    x = srabs*wi_double[idx];
    if (rabs < ki) return x;
  }
}

ALWAYS_INLINE double draw_normref(randompack_rng *rng, int *w) {
  uint64_t rabs, ki, r;
  double x;
  uint8_t idx;
  r = draw_u64(rng, w);
  x = norm_likely_ref(r, &idx, &ki, &rabs);
  if (LIKELY(rabs < ki)) return x;
  x = norm_unlikely_ref(rng, w, x, idx, rabs, ki);
  return x;
}

NEVER_INLINE void rand_normref(double *buf, size_t len, randompack_rng *rng) {
  align64(rng);
  draw_raw_copy((uint8_t *)buf, len*sizeof(uint64_t), rng);
  int w = enter_u64_mode(rng), sign;
  uint8_t idx;
  uint64_t r, rabs, ki;
  int64_t srabs;
  double x;
  for (size_t i = len; i-- > 0;) {
    uint8_t *bufpi = (uint8_t*)(buf + i);
    memcpy(&r, bufpi, sizeof(r));
    TOP_BYTE(idx, buf[i]);
    ki = ki_double[idx];
    sign = (r >> 52) & 1;
    rabs = r & 0x000fffffffffffff;
    srabs = sign ? -(int64_t)rabs : (int64_t)rabs;
    x = srabs*wi_double[idx];
    buf[i] = LIKELY(rabs < ki) ? x : norm_unlikely_ref(rng, &w, x, idx, rabs, ki);
  }
  exit_u64_mode(rng, w);
}
