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

static float tail_draw_f(randompack_rng *rng, uint32_t rabs, int *w) {
  float xx, yy;
  for (;;) {
    // Switch to 1.0 - U to avoid log(0.0), see GH 13361
    xx = -ziggurat_nor_inv_r_f * log1pf(-u01f(rng, w));
    yy = -log1pf(-u01f(rng, w));
    if (yy + yy > xx * xx)
      return ((rabs >> 8) & 0x1) ? -(ziggurat_nor_r_f + xx)
                                 : ziggurat_nor_r_f + xx;
  }
}

ALWAYS_INLINE float draw_normf(randompack_rng *rng, int *w) {
  uint32_t r;
  int sign;
  uint32_t rabs;
  int idx;
  float x;
  for (;;) {
    r = draw_u32_nocheck(rng, w);
    idx = r & 0xff;
    r >>= 8;
    sign = r & 0x1;
    rabs = (r >> 1) & 0x0007fffff;
    x = rabs * wi_float[idx];
    x = flip_sign_if_f(x, sign);
    if (LIKELY(rabs < ki_float[idx])) {
      return x;
    }
    else if (UNLIKELY(idx == 0)) {
      x = tail_draw_f(rng, rabs, w);
      make_half_full32(rng, w);
      return x;
    }
    else {
      float h = fi_float[idx - 1] - fi_float[idx];
      float y = u01f(rng, w);
      if (h*y + fi_float[idx] < expf(-x*x/2)) {
        make_half_full32(rng, w);
        return x;
      }
    }
    make_half_full32(rng, w);
  }
}

ALWAYS_INLINE void rand_normf(float *buf, size_t len, randompack_rng *rng) {
  int w = enter_u32_mode(rng);
  size_t i = 0;
  while (i < len) {
    make_half_full32(rng, &w);
    int take = min(len - i, BUFSIZE);
    for (int j = 0; j < take; j++) {
      buf[i + j] = draw_normf(rng, &w);
    }
    i += take;
  }
  exit_u32_mode(rng, w);
}

#ifndef U01_HIRES
ALWAYS_INLINE float standard_exponential_unlikely_f(randompack_rng *rng,
  uint8_t idx, float x, int *w) {
  while (true) { // added loop to replace recursion, to allow speedup.
    if (idx == 0) {
      // Switch to 1.0 - U to avoid log(0.0), see GH 13361
      return ziggurat_exp_r_f - log1pf(-u01f(rng, w));
    }
    else if ((fe_float[idx-1] - fe_float[idx])*u01f(rng, w) + fe_float[idx] <
             expf(-x)) {
      return x;
    }
    else {
      uint32_t ri = draw_u32(rng, w);
      ri >>= 1;
      idx = ri & 0xFF;
      ri >>= 8;
      x = ri*we_float[idx];
      if (ri < ke_float[idx]) {
        return x;
      }
    }
  }
}

ALWAYS_INLINE float draw_expf(randompack_rng *rng, int *w) {
  uint32_t ri;
  uint8_t idx;
  float xi;
  ri = draw_u32_nocheck(rng, w);
  ri >>= 1;
  idx = ri & 0xFF;
  ri >>= 8;
  xi = ri * we_float[idx];
  if (ri >= ke_float[idx]) {
    xi = standard_exponential_unlikely_f(rng, idx, xi, w);
    make_half_full32(rng, w);
  }
  return xi;
}

ALWAYS_INLINE void rand_expf(float *buf, size_t len, randompack_rng *rng) {
  int w = enter_u32_mode(rng);
  size_t i = 0;
  while (i < len) {
    make_half_full32(rng, &w);
    int take = min(len - i, BUFSIZE);
    for (size_t j = i; j < i + take; j++) {
      buf[j] = draw_expf(rng, &w);
    }
    i += take;
  }
  exit_u32_mode(rng, w);
}
#else
// This version takes the index from the high bits and uses 24 bits for x. Because of that
// wef must be halved and kef doubled; gensig.c constructs zig_new.h with these.
#include "zig_new.h"
ALWAYS_INLINE void rand_expf(float *x, size_t len, randompack_rng *rng) {
  int w = enter_u32_mode(rng);
  uint32_t ri;
  uint8_t idx;
  float xi;
  size_t i = 0;
  while (i < len) {
    ri = draw_u32(rng, &w);
    idx = ri >> 24;
    ri &= 0x00FFFFFFu;
    xi = ri*wehf[idx];            // wehf = we_float/2
    if (ri < ke2f[idx])           // ke2f = kef*2
      x[i++] = xi;                // fast accept
    else if (idx == 0)            // tail
      x[i++] = ziggurat_exp_r_f - log1pf(-u01f(rng, &w));
    else if ((fe_float[idx-1]-fe_float[idx])*u01f(rng, &w) + fe_float[idx] < expf(-xi))
      x[i++] = xi;                // wedge accept
  }
  exit_u32_mode(rng, w);
}
#endif
