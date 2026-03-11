// -*- C -*-
#include <string.h>
#include "ziggurat_const_float.h"
#define FLT_SIGNBIT 0x80000000u
ALWAYS_INLINE float flip_sign_if_f(float x, int sign) {
  uint32_t bits, mask = (uint32_t)0 - (uint32_t)(sign & 1);
  mask &= FLT_SIGNBIT;
  memcpy(&bits, &x, sizeof bits);
  bits ^= mask;
  memcpy(&x, &bits, sizeof bits);
  return x;
}

static float norm_tail_draw_f(randompack_rng *rng, uint32_t rabs, float y, int *w) {
  float xx, yy, x;
  for (;;) {
    // Switch to 1.0 - U to avoid log(0.0), see GH 13361
    x = draw_u01f(rng, w);
    xx = -ziggurat_nor_inv_r_f*openlibm_log1pf(-x);
    yy = -openlibm_log1pf(-y);
    if (yy + yy > xx * xx)
      return ((rabs >> 8) & 0x1) ? -(ziggurat_nor_r_f + xx) : ziggurat_nor_r_f + xx;
    y = draw_u01f(rng, w);
  }
}

ALWAYS_INLINE float normf_likely(randompack_rng *rng, int *w, int *idx,
  uint32_t *ki, uint32_t *rabs) {
  uint32_t r;
  int sign;
  float x;
  if (UNLIKELY(*w >= 2*BUFSIZE)) {
    *w = rng->buf_word = 0;
    rng->fill(rng->buf.u64, BUFSIZE, &rng->state);
  }
  r = rng->buf.u32[(*w)++];
  *idx = r & 0xff;
  r >>= 8;
  *ki = ki_float[*idx];
  sign = r & 0x1;
  *rabs = (r >> 1) & 0x007fffff;
  x = (*rabs)*wi_float[*idx];
  x = flip_sign_if_f(x, sign);
  return x;
}

ALWAYS_INLINE float normf_unlikely(randompack_rng *rng, int *w, float x,
  int idx, uint32_t *rabs, uint32_t ki) {
  float y, f0, f1, tangent;
  uint32_t dr, dk;
  while (true) {
    y = draw_u01f(rng, w);
    dr = (1U << 23) - *rabs;
    tangent = dr*slope_normf[idx];
    if (LIKELY(idx > 52)) { // idx == 52 straddles the inflection at x=1
      if (y <= tangent) return x;    // below tangent
      dk = (1U << 23) - ki;
      if (y*dk > dr) goto next_try;     // above secant
    }
    else if (UNLIKELY(idx == 0)) {      // in tail
      x = norm_tail_draw_f(rng, *rabs, y, w);
      return x;
    }
    else if (LIKELY(idx < 52)) {
      if (y > tangent) goto next_try;   // above tangent
      dk = (1U << 23) - ki;
      if (y*dk <= dr) return x;      // below secant
    }
    f0 = fi_float[idx-1];
    f1 = fi_float[idx];
    if (f1 + y*(f0 - f1) < openlibm_expf(-x*x/2)) return x; // below curve
  next_try:
    x = normf_likely(rng, w, &idx, &ki, rabs);
    if (*rabs < ki) return x;
  }
}

ALWAYS_INLINE void rand_normf(float *buf, size_t len, randompack_rng *rng) {
  int w = enter_u32_mode(rng);
  size_t i = 0;
  while (i < len) {
    size_t rem = len - i;
    uint32_t rabs, ki;
    float x;
    int idx, take;
    take = rem < 2*BUFSIZE ? (int)rem : 2*BUFSIZE;
    for (int j = 0; j < take; j++) {
      x = normf_likely(rng, &w, &idx, &ki, &rabs);
      if (LIKELY(rabs < ki)) {
        buf[i + j] = x;
        continue;
      }
      buf[i + j] = normf_unlikely(rng, &w, x, idx, &rabs, ki);
    }
    i += take;
  }
  exit_u32_mode(rng, w);
}

ALWAYS_INLINE float draw_expf(randompack_rng *rng, int *w) {
  float x;
  for (;;) {
    float f0, f1;
    uint32_t r = draw_u32(rng, w);
    r >>= 1;
    uint8_t idx = r & 0xFF;
    r >>= 8;
    uint32_t ki = ke_float[idx];
    x = r*we_float[idx];;
    if (LIKELY(r < ki)) return x;     // in the left rectangle
    if (UNLIKELY(idx == 0)) {         // in the tail
      x = ziggurat_exp_r_f - openlibm_log1pf(-draw_u01f(rng, w));
      goto accept;
    }
    float y = draw_u01f(rng, w);
    uint32_t dr = (1U << 24) - r;
    float tangent = dr*slope_expf[idx];
    if (y <= tangent) goto accept;    // below tangent
    uint32_t dk = (1U << 24) - ki;
    if (y*dk > dr) continue;     // above secant
    f0 = fe_float[idx-1];
    f1 = fe_float[idx];
    if (f1 + y*(f0 - f1) < openlibm_expf(-x)) goto accept; // below curve
  }
accept:
  return x;
}

ALWAYS_INLINE void rand_expf(float *buf, size_t len, randompack_rng *rng) {
  int w = enter_u32_mode(rng);
  size_t i = 0;
  while (i < len) {
    int take = min(len - i, 2*BUFSIZE);
    for (size_t j = i; LIKELY(j < i + take); j++) {
      buf[j] = draw_expf(rng, &w);
    }
    i += take;
  }
  exit_u32_mode(rng, w);
}
