// -*- C -*-
// Float version matching norm_exp.inc structure exactly

#define MUL32_WIDE(a, b, hi, lo) do {                          \
  uint64_t _p = ((uint64_t)(a)) * ((uint64_t)(b));             \
  (lo) = (uint32_t)_p;                                         \
  (hi) = (uint32_t)(_p >> 32);                                 \
} while (0)

ALWAYS_INLINE float norm_decodef(uint32_t r, uint8_t idx, uint32_t *ki, uint32_t *rabs) {
  *ki = ki_float[idx];
  *rabs = r & 0x007fffff;
  int32_t sign = -((int32_t)((r >> 23) & 1));
  int32_t s = ((int32_t)*rabs ^ sign) - sign;  // branchless +/- rabs
  return (float)s * wi_float[idx];
}

//=============================== NORMAL DISTRIBUTION ==============================

NEVER_INLINE bool below_normpdf_f(uint8_t idx, float x, float y) {
  float f0 = fi_float[idx-1];
  float f1 = fi_float[idx];
  return f1 + y*(f0 - f1) < openlibm_expf(-x*x/2);
}

NEVER_INLINE float norm_tail_draw_f(randompack_rng *rng, uint32_t rabs, float y,
  int *w) {
  float xx, yy, x;
  for (;;) {
    /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
    x = draw_u01f(rng, w);
    xx = -ziggurat_nor_inv_r_f * openlibm_log1pf(-x);
    yy = -openlibm_log1pf(-y);
    if (LIKELY(yy + yy > xx * xx))
      return ((rabs >> 8) & 0x1) ? -(ziggurat_nor_r_f + xx) : ziggurat_nor_r_f + xx;
    y = draw_u01f(rng, w);
  }
}

NEVER_INLINE float avx512_unlikely(randompack_rng *rng, int *w, float x,
                                    uint8_t idx, uint32_t rabs, uint32_t ki) {
  for (;;) {
    float u = draw_u01f(rng, w);
    if (UNLIKELY(idx == 0)) 
      return norm_tail_draw_f(rng, rabs, u, w);
    else if (below_normpdf_f(idx, x, u))
      return x;
    uint32_t ru = draw_u32(rng, w);
    idx = ru >> 24;
    x = norm_decodef(ru, idx, &ki, &rabs);
    if (LIKELY(rabs < ki)) return x;
  }
}

NEVER_INLINE float normf_unlikely(randompack_rng *rng, int *w, float x,
  uint8_t idx, uint32_t rabs, uint32_t ki) {
  uint32_t ru, tmp, L, R, Y, YL;
  const int inflection_idx = 52;
  for (;;) {
    Y = draw_u32(rng, w);
    if (UNLIKELY(idx == 0)) {
		float u = U32_TO_FLOAT(Y);
      x = norm_tail_draw_f(rng, rabs, u, w);
      return x;
    }
    if (!rng->cpu_has_avx512) {
		L = (1U << 23) - ki;
		R = (1U << 23) - rabs;
      MUL32_WIDE(Y, L, YL, tmp);
      (void)tmp;
      if (LIKELY(idx > inflection_idx)) {
        if (YL > R) goto next_try;
        if (YL + Dnormf[idx] < R) return x;
      }
      else if (LIKELY(idx < inflection_idx)) {
        if (YL < R) return x;
        if (YL > R + Dnormf[idx]) goto next_try;
      }
      else {
        if (YL > R + Dzig52f) goto next_try;
        if (YL + Dnormf[idx] < R) return x;
      }
    }
	 float u = U32_TO_FLOAT(Y);
    if (below_normpdf_f(idx, x, u)) return x;
  next_try:
    ru = draw_u32(rng, w);
    idx = ru >> 24;
    x = norm_decodef(ru, idx, &ki, &rabs);
    if (LIKELY(rabs < ki)) return x;
  }
}

ALWAYS_INLINE float draw_normf(randompack_rng *rng, int *w) {
  uint32_t rabs, ki, r;
  float x;
  uint8_t idx;
  r = draw_u32(rng, w);
  idx = r >> 24;
  x = norm_decodef(r, idx, &ki, &rabs);
  if (LIKELY(rabs < ki)) return x;
  if (rng->cpu_has_avx512) x = avx512_unlikely(rng, w, x, idx, rabs, ki);
  else                     x = normf_unlikely(rng, w, x, idx, rabs, ki);
  return x;
}

NEVER_INLINE void rand_normf(float *buf, size_t len, randompack_rng *rng) {
  int w = enter_u32_mode(rng);
  draw_raw_copy32((uint32_t *)buf, len, rng, &w);
  uint8_t idx;
  uint32_t r, rabs, ki;
  float x;
  for (size_t i = len; i-- > 0;) {
    uint8_t *bufpi = (uint8_t*)(buf + i);
    memcpy(&r, bufpi, sizeof(r));
    memcpy(&idx, (uint8_t*)(buf + i) + 3, 1);  // measured slightly faster than >> 24
    x = norm_decodef(r, idx, &ki, &rabs);
    if (UNLIKELY(rabs > ki)) {
      if (rng->cpu_has_avx512) x = avx512_unlikely(rng, &w, x, idx, rabs, ki);
      else                     x = normf_unlikely(rng, &w, x, idx, rabs, ki);
    }
    buf[i] = x;
  }
  exit_u32_mode(rng, w);
}

//========================= exponential distribution ===========================

NEVER_INLINE bool below_exppdf_f(uint8_t idx, float x, float y) {
  // Use openlibm_expf to ensure cross-platform compatibility (with expf we
  // the result of the test might depend on the platform)
  float f0, f1;
  f0 = fe_float[idx-1];
  f1 = fe_float[idx];
  return (f1 + y*(f0 - f1) < openlibm_expf(-x));
}

ALWAYS_INLINE float exp_taildraw_f(randompack_rng *rng, int *w) {
  float x = ziggurat_exp_r_f - openlibm_log1pf(-draw_u01f(rng, w));
  return x;
}

ALWAYS_INLINE float expf_unlikely(randompack_rng *rng, int *w, float x, uint8_t idx,
  uint32_t r) {
  float y;
  uint32_t Y, YL, R, L, tmp, ke = ke_float[idx];
  while (true) {
    if (UNLIKELY(idx == 0)) return exp_taildraw_f(rng, w);
    Y = draw_u32(rng, w);
    L = (1U << 23) - ke;
    R = (1U << 23) - r;
    MUL32_WIDE(Y, L, YL, tmp);
    (void)tmp;
    if (LIKELY(YL > R)) goto next_try;
    if (YL + Dexpf[idx] < R) return x;
    y = U32_TO_FLOAT(Y);
    if (below_exppdf_f(idx, x, y)) return x;
  next_try:
    r = draw_u32(rng, w);
    idx = r >> 24;
    r &= 0x007fffff;
    ke = ke_float[idx];
    x = r*we_float[idx];
    if (LIKELY(r < ke)) return x;
  }
}

ALWAYS_INLINE void rand_expf(float *buf, size_t len, randompack_rng *rng) {
  align64(rng);
  draw_raw_copy((uint8_t *)buf, len*sizeof(uint32_t), rng);
  int w = enter_u32_mode(rng);
  uint8_t idx;
  uint32_t r;
  float x;
  for (size_t i = len; i-- > 0;) {
    uint8_t *bufpi = (uint8_t*)(buf + i);
    memcpy(&r, bufpi, sizeof(r));
    memcpy(&idx, (uint8_t*)(buf + i) + 3, 1);  // measured slightly faster than >> 24
    r &= 0x007fffff;
    x = r*we_float[idx];
    buf[i] = LIKELY(r < ke_float[idx]) ? x : expf_unlikely(rng, &w, x, idx, r);
  }
  exit_u32_mode(rng, w);
}
