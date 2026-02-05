// -*- C -*-
// Code copied from NumPy: github.com/numpy/numpy/tree/f6440be7b8,
//   numpy/random/src/distributions/distributions.c.
// and adapted to randompack. No change to computations.
// Copyright (c) NumPy Developers.


#define DBL_SIGNBIT 0x8000000000000000ULL

#include <stdint.h>
#include <math.h>
#include <string.h>
#include "printX.h"
#include "randompack.h"
#include "ziggurat_constants.h"
#undef NDEBUG
#include "assert.h"

static double norm_tail_draw(randompack_rng *rng, uint64_t rabs, int *w) {
  double xx, yy;
  for (;;) {
    /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
    xx = -ziggurat_nor_inv_r * log1p(-draw_u01(rng, w));
    yy = -log1p(-draw_u01(rng, w));
    if (LIKELY(yy + yy > xx * xx))
      return ((rabs >> 8) & 0x1) ? -(ziggurat_nor_r + xx) : ziggurat_nor_r + xx;
  }
}

#define DBL_SIGNBIT 0x8000000000000000ULL
ALWAYS_INLINE double flip_sign_if(double x, int sign) {
  uint64_t bits, mask = (uint64_t)0 - (uint64_t)(sign & 1);
  mask &= DBL_SIGNBIT;
  memcpy(&bits, &x, sizeof bits);
  bits ^= mask;
  memcpy(&x, &bits, sizeof bits);
  return x;
}

ALWAYS_INLINE double draw_norm(randompack_rng *rng, int *w) {
  uint64_t rabs, r;
  int sign, idx;
  double x;
  for (;;) {
    r = draw_u64_nocheck(rng, w);
    idx = r & 0xff;
    r >>= 8;
    sign = r & 0x1;
    rabs = (r >> 1) & 0x000fffffffffffff;
    x = rabs*wi_double[idx];
    x = flip_sign_if(x, sign);
    if (LIKELY(rabs < ki_double[idx])) {
      return x;
    }
    if (UNLIKELY(idx == 0)) {
      x = norm_tail_draw(rng, rabs, w);
      make_half_full(rng, w);
      return x;
    }
    double h = fi_double[idx - 1] - fi_double[idx];
    double y = draw_u01(rng, w);
    if (h*y + fi_double[idx] < exp(-0.5*x*x)) {
      make_half_full(rng, w);
      return x;
    }
    make_half_full(rng, w);
  }
}

ALWAYS_INLINE void rand_norm(double *buf, size_t len, randompack_rng *rng) {
  int w = enter_u64_mode(rng);
  size_t i = 0;
  while (i < len) {
    make_half_full(rng, &w);
    int take = min(len - i, BUFSIZE/2);
    for (int j = 0; j < take; j++) {
      buf[i + j] = draw_norm(rng, &w);
    }
    i += take;
  }
  exit_u64_mode(rng, w);
}

// Following four functions are copied from NumPy, except for marked changes
ALWAYS_INLINE double standard_exponential_unlikely(
  randompack_rng *rng, uint8_t idx, uint64_t ki, double wi, uint64_t r, double x, int *w) {
  while (true) { // added loop to replace recursion, to allow speedup.
    if (UNLIKELY(idx == 0)) {
      /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
      return ziggurat_exp_r - log1p(-draw_u01(rng, w));
    }
    else {
      double y = draw_u01(rng, w);
		uint64_t r0 = (1ULL << 53) - ki;
		uint64_t r1 = r - ki;
		if (r0*y > r1) {
		  double f1 = fe_double[idx-1], f0 = fe_double[idx];
		  double yh = y*(f1 - f0);
		  if (yh >= r1*wi*f1) return x;
		  if (UNLIKELY(f1 - yh < exp(-x))) return x;
		}
      r = draw_u64(rng, w);
      r >>= 3;
      idx = r & 0xFF;
      r >>= 8;
      wi = we_double[idx];
      x = r*wi;
      ki = ke_double[idx];
      if (LIKELY(r < ki)) return x;
    }
  }
}

ALWAYS_INLINE double draw_exp(randompack_rng *rng, int *w) {
  while (true) {
    uint64_t r = draw_u64_nocheck(rng, w);
    r >>= 3;
    uint8_t idx = r & 0xFF;
    r >>= 8;
    uint64_t ki = ke_double[idx];
    double wi = we_double[idx];
    double x = r*wi;
    if (LIKELY(r < ki)) return x;
    if (UNLIKELY(idx == 0)) {
      return ziggurat_exp_r - log1p(-draw_u01(rng, w));
    }
    double y = draw_u01(rng, w);
    uint64_t r0 = (1ULL << 53) - ki;
    uint64_t r1 = r - ki;
    if (r0*y > r1) { // below secant
      double f1 = fe_double[idx-1], f0 = fe_double[idx];
      double yh = y*(f1 - f0);
      if (yh >= r1*wi*f1 ||    // below tangent
          f1 - yh < exp(-x)) { // or below pdf
        make_half_full(rng, w);
        return x;
      }
    }
    make_half_full(rng, w);
  }
}

ALWAYS_INLINE void rand_exp(double *buf, size_t len, randompack_rng *rng) {
  int w = enter_u64_mode(rng);
  size_t i = 0;
  while (i < len) {
    make_half_full(rng, &w);
    int take = min(len - i, BUFSIZE/2);
    for (size_t j = i; LIKELY(j < i + take); j++) {
      buf[j] = draw_exp(rng, &w);
    }
    i += take;
  }
  exit_u64_mode(rng, w);
}

#include "norm_exp_float.inc"
