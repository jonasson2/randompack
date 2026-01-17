// -*- C -*-
// Code copied from NumPy: github.com/numpy/numpy/tree/f6440be7b8,
//   numpy/random/src/distributions/distributions.c.
// Copyright (c) NumPy Developers.
// Licensed under the BSD-3-Clause license; see LICENSE.txt.
//
// Only change exp-->expf in the float version (likely a NumPy oversight)
//
// Macros for randompack compatibility:
#define bitgen_t randompack_rng
#define next_uint32 draw_u32_fast
#define next_uint64 draw_u64
#define next_float(rng) U32_TO_FLOAT(draw_u32_fast(rng))
#define next_double(rng) U64_TO_DOUBLE(draw_u64(rng))
#define npy_log1p log1p
#define npy_log1pf log1pf

#include <stdint.h>
#include <math.h>
#include "randompack.h"
#include "ziggurat_constants.h"

ALWAYS_INLINE static double random_standard_normal(bitgen_t *bitgen_state) {
  uint64_t r;
  int sign;
  uint64_t rabs;
  int idx;
  double x, xx, yy;
  for (;;) {
    /* r = e3n52sb8 */
    r = next_uint64(bitgen_state);
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
        xx = -ziggurat_nor_inv_r * npy_log1p(-next_double(bitgen_state));
        yy = -npy_log1p(-next_double(bitgen_state));
        if (yy + yy > xx * xx)
          return ((rabs >> 8) & 0x1) ? -(ziggurat_nor_r + xx)
                                     : ziggurat_nor_r + xx;
      }
    } else {
      if (((fi_double[idx - 1] - fi_double[idx]) * next_double(bitgen_state) +
           fi_double[idx]) < exp(-0.5 * x * x))
        return x;
    }
  }
}

ALWAYS_INLINE static void rand_norm(double *buf, size_t len, bitgen_t *bitgen_state) {
  // align64(bitgen_state);
  for (size_t i = 0; i < len; i++) {
    buf[i] = random_standard_normal(bitgen_state);
  }
}

// Following four functions are copied from NumPy, except for marked changes
ALWAYS_INLINE static double standard_exponential_unlikely(bitgen_t *bitgen_state,
																					  uint8_t idx, double x) {
  while (true) { // added loop to replace recursion, to allow speedup.
    if (idx == 0) {
      /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
      return ziggurat_exp_r - npy_log1p(-next_double(bitgen_state));
    }
    else if ((fe_double[idx - 1] - fe_double[idx])*next_double(bitgen_state) +
             fe_double[idx] < exp(-x)) {
      return x;
    }
    else { 
      // Replaced calling rand_exp with the following:
      uint64_t ri = next_uint64(bitgen_state);
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

ALWAYS_INLINE static void rand_exp(double *x, size_t len, bitgen_t *bitgen_state) {
  // align64(bitgen_state);
  for (size_t i=0; i<len; i++) {
    uint64_t ri;
	 uint8_t idx;
	 double xi;
	 ri = next_uint64(bitgen_state);
    ri >>= 3;
    idx = ri & 0xFF;
    ri >>= 8;
    xi = ri * we_double[idx];
    if (ri < ke_double[idx]) {
      x[i] = xi;
	 }
    else {
      x[i] = standard_exponential_unlikely(bitgen_state, idx, xi);
	 }
  }
}
// #include "norm_exp_float.inc"
