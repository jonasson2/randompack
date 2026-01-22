// -*- c -*-
#include <string.h>
#include "ziggurat_const_float.h"
#define FLT_SIGNBIT 0x80000000u
ALWAYS_INLINE static float flip_sign_if_f(float x, int sign) {
  uint32_t bits, mask = (uint32_t)0 - (uint32_t)(sign & 1);
  mask &= FLT_SIGNBIT;
  memcpy(&bits, &x, sizeof bits);
  bits ^= mask;
  memcpy(&x, &bits, sizeof bits);
  return x;
}

ALWAYS_INLINE static float u01f(randompack_rng *rng) {
  return U32_TO_FLOAT(draw_u32_fast(rng));
}

ALWAYS_INLINE static float draw_normf(randompack_rng *rng) {
  uint32_t r, rabs;
  int sign, idx;
  float x, xx, yy;
  for (;;) {
    r = next_uint32(rng);
    idx = (int)(r & 0xff);
    r >>= 8;
    sign = (int)(r & 0x1);
    rabs = (r >> 1) & 0x0007fffff;
    x = (float)rabs*wi_float[idx];
    x = flip_sign_if_f(x, sign);
    if (rabs < ki_float[idx]) {
      return x;
    }
    if (idx == 0) {
      for (;;) {
        // Switch to 1.0 - U to avoid log(0.0)
        xx = -ziggurat_nor_inv_r_f*npy_log1pf(-u01f(rng));
        yy = -npy_log1pf(-u01f(rng));
        if (yy + yy > xx*xx)
          return ((rabs >> 8) & 0x1) ? -(ziggurat_nor_r_f + xx)
                                     :  (ziggurat_nor_r_f + xx);
      }
    }
    else if (((fi_float[idx - 1] - fi_float[idx])*u01f(rng) + fi_float[idx]) <
        expf(-0.5f*x*x)) {          // strip rejection test
      return x;
    }
  }
}

ALWAYS_INLINE static void rand_normf(float *buf, size_t len, randompack_rng *rng) {
  enter_u32_mode(rng);
  for (size_t i = 0; i < len; i++) buf[i] = draw_normf(rng);
  exit_u32_mode(rng);
}

ALWAYS_INLINE static void rand_normalf(float *buf, size_t len, float mu, float sigma,
                                       randompack_rng *rng) {
  rand_normf(buf, len, rng);
  if (mu != 0 || sigma != 1)
    for (size_t i = 0; i < len; i++) buf[i] = mu + sigma*buf[i];
}
#ifndef U01_HIRES
ALWAYS_INLINE static float standard_exponential_unlikely_f(randompack_rng *rng,
                                                           uint8_t idx, float x) {
  while (true) { // added loop to replace recursion
    if (idx == 0) {
      /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
      return ziggurat_exp_r_f - npy_log1pf(-next_float(rng));
    }
    else if ((fe_float[idx - 1] - fe_float[idx])*u01f(rng) +
             fe_float[idx] < expf(-x)) {
      return x;
    }
    else {
      // Replaced calling rand_expf with the following:
      uint32_t ri = draw_u32_fast(rng);
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

ALWAYS_INLINE static void rand_expf(float *x, size_t len, randompack_rng *rng) {
  enter_u32_mode(rng);
  for (size_t i=0; i<len; i++) {
    uint32_t ri;
    uint8_t idx;
    float xi;
    ri = next_uint32(rng);
    ri >>= 1;
    idx = ri & 0xFF;
    ri >>= 8;
    xi = ri * we_float[idx];
    if (ri < ke_float[idx]) { // 98.9% of the time
      x[i] = xi;
    }
    else {
      x[i] = standard_exponential_unlikely_f(rng, idx, xi);
    }
  }
  exit_u32_mode(rng);
}

#else
// This version takes the index from the high bits and uses 24 bits for x. Because of that
// wef must be halved and kef doubled; gensig.c constructs zig_new.h with these.
#include "zig_new.h"
ALWAYS_INLINE static void rand_expf(float *x, size_t len, randompack_rng *rng) {
  enter_u32_mode(rng);
  uint32_t ri;
  uint8_t idx;
  float xi;
  size_t i = 0;
  while (i < len) {
    ri = draw_u32_fast(rng);
    idx = ri >> 24;
    ri &= 0x00FFFFFFu;
    xi = ri*wehf[idx];            // wehf = we_float/2
    if (ri < ke2f[idx])           // ke2f = kef*2
      x[i++] = xi;                // fast accept
    else if (idx == 0)            // tail
      x[i++] = ziggurat_exp_r_f - log1pf(-next_float(rng));
    else if ((fe_float[idx-1]-fe_float[idx])*next_float(rng) + fe_float[idx] < expf(-xi))
      x[i++] = xi;                // wedge accept
  }
  exit_u32_mode(rng);
}
#endif
