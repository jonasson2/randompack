// -*- C -*-
// Code copied verbatim from NumPy: github.com/numpy/numpy/tree/f6440be7b8,
//   numpy/random/src/distributions/distributions.c.
// Copyright (c) NumPy Developers.
// Licensed under the BSD-3-Clause license; see LICENSE.txt.
//
// Only change exp-->expf in the float version (likely a NumPy oversight)
//
// Macros for randompack compatibility:
#define bitgen_t randompack_rng
#define next_uint32 draw_u32
#define next_uint64 draw_u64
#define next_float(rng) ((draw_u32(rng) >> 8) * (float)0x1.0p-24)
#define next_double(rng) ((draw_u64(rng) >> 11) * 0x1.0p-53)
#define npy_log1p log1p
#define npy_log1pf log1pf

#include <stdint.h>
#include <math.h>
#include "randompack.h"
#include "ziggurat_constants.h"

static inline double random_standard_normal(bitgen_t *bitgen_state) {
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

static inline float random_standard_normal_f(bitgen_t *bitgen_state) {
  uint32_t r;
  int sign;
  uint32_t rabs;
  int idx;
  float x, xx, yy;
  for (;;) {
    /* r = n23sb8 */
    r = next_uint32(bitgen_state);
    idx = r & 0xff;
    sign = (r >> 8) & 0x1;
    rabs = (r >> 9) & 0x0007fffff;
    x = rabs * wi_float[idx];
    if (sign & 0x1)
      x = -x;
    if (rabs < ki_float[idx])
      return x; /* # 99.3% of the time return here */
    if (idx == 0) {
      for (;;) {
        /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
        xx = -ziggurat_nor_inv_r_f * npy_log1pf(-next_float(bitgen_state));
        yy = -npy_log1pf(-next_float(bitgen_state));
        if (yy + yy > xx * xx)
          return ((rabs >> 8) & 0x1) ? -(ziggurat_nor_r_f + xx)
                                     : ziggurat_nor_r_f + xx;
      }
    } else {
      if (((fi_float[idx - 1] - fi_float[idx]) * next_float(bitgen_state) +
           fi_float[idx]) < expf(-0.5 * x * x)) // changed from exp in NumPy (Kristján)
        return x;
    }
  }
}

