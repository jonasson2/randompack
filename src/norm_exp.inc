// -*- C -*-
// Code copied from NumPy: github.com/numpy/numpy/tree/f6440be7b8,
//   numpy/random/src/distributions/distributions.c.
// Copyright (c) NumPy Developers.
// Licensed under the BSD-3-Clause license; see LICENSE.txt.
//
// Only change exp-->expf in the float version (likely a NumPy oversight)
//
// Macros for randompack compatibility:
//#define randompack_rng randompack_rng
#define next_uint32 draw_u32_fast
//#define next_uint64 draw_u64
#define bitgen_t randompack_rng
#define next_float(rng) U32_TO_FLOAT(draw_u32_fast(rng))
#define next_double(rng) U64_TO_DOUBLE(draw_u64(rng))
//#define npy_log1p log1p
#define npy_log1pf log1pf

#include <stdint.h>
#include <math.h>
#include "randompack.h"
#include "ziggurat_constants.h"

ALWAYS_INLINE static double random_standard_normal(randompack_rng *rng) {
  uint64_t r;
  int sign;
  uint64_t rabs;
  int idx;
  double x, xx, yy;
  for (;;) {
    /* r = e3n52sb8 */
    r = draw_u64(rng);
    idx = r & 0xff;
    r >>= 8;
    sign = r & 0x1;
    rabs = (r >> 1) & 0x000fffffffffffff;
    x = rabs * wi_double[idx];
    if (sign & 0x1)
      x = -x;
    if (rabs < ki_double[idx])
      return x; /* 99.3% of the time return here */
    if (idx == 0) {
      for (;;) {
        /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
        xx = -ziggurat_nor_inv_r * log1p(-next_double(rng));
        yy = -log1p(-next_double(rng));
        if (yy + yy > xx * xx)
          return ((rabs >> 8) & 0x1) ? -(ziggurat_nor_r + xx)
                                     : ziggurat_nor_r + xx;
      }
    } else {
      if (((fi_double[idx - 1] - fi_double[idx]) * next_double(rng) +
           fi_double[idx]) < exp(-0.5 * x * x))
        return x;
    }
  }
}

ALWAYS_INLINE static double u01(randompack_rng *rng, int *w) {
  return U64_TO_DOUBLE(draw_u64_fast(rng, w));
}

static double tail_draw(randompack_rng *rng, uint64_t rabs, int *w) {
  double xx, yy;
  for (;;) {
    /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
    xx = -ziggurat_nor_inv_r * log1p(-u01(rng, w));
    yy = -log1p(-u01(rng, w));
    if (yy + yy > xx * xx)
      return ((rabs >> 8) & 0x1) ? -(ziggurat_nor_r + xx)
        : ziggurat_nor_r + xx;
  }
}

ALWAYS_INLINE static void rand_norm(double *buf, size_t len, randompack_rng *rng) {
  uint64_t r;
  int sign;
  uint64_t rabs;
  int idx;
  double x;
  int w = enter_u64_mode(rng);
  for (size_t i = 0; i < len; i++) {
    for (;;) {
      r = draw_u64_fast(rng, &w);
      idx = r & 0xff;
      r >>= 8;
      sign = r & 0x1;
      rabs = (r >> 1) & 0x000fffffffffffff;
      x = rabs * wi_double[idx];
      if (sign & 0x1)
        x = -x;
      if (rabs < ki_double[idx]) {
        buf[i] = x;
        break;
      }
      if (idx == 0) {
        buf[i] = tail_draw(rng, rabs, &w);
        break;
      }
      else {
        double h = fi_double[idx - 1] - fi_double[idx];
        if (h*u01(rng, &w) + fi_double[idx] < exp(-x*x/2)) {
          buf[i] = x;
          break;
        }
      }
    }
  }
  exit_u64_mode(rng, w);
}

// Following four functions are copied from NumPy, except for marked changes
ALWAYS_INLINE static double standard_exponential_unlikely(randompack_rng *rng,
																					  uint8_t idx, double x) {
  while (true) { // added loop to replace recursion, to allow speedup.
    if (idx == 0) {
      /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
      return ziggurat_exp_r - log1p(-next_double(rng));
    }
    else if ((fe_double[idx - 1] - fe_double[idx])*next_double(rng) +
             fe_double[idx] < exp(-x)) {
      return x;
    }
    else { 
      // Replaced calling rand_exp with the following:
      uint64_t ri = draw_u64(rng);
      ri >>= 3;
      idx = ri & 0xFF;
      ri >>= 8;
      x = ri*we_double[idx];
      if (ri < ke_double[idx]) {
        return x;
      }
    }
  }
}

ALWAYS_INLINE static void rand_exp(double *x, size_t len, randompack_rng *rng) {
  // align64(rng);
  for (size_t i=0; i<len; i++) {
    uint64_t ri;
	 uint8_t idx;
	 double xi;
	 ri = draw_u64(rng);
    ri >>= 3;
    idx = ri & 0xFF;
    ri >>= 8;
    xi = ri * we_double[idx];
    if (ri < ke_double[idx]) {
      x[i] = xi;
	 }
    else {
      x[i] = standard_exponential_unlikely(rng, idx, xi);
	 }
  }
}
#include "norm_exp_float.inc"
