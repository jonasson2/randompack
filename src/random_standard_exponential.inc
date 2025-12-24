// -*- C -*-
// Code copied verbatim from NumPy: github.com/numpy/numpy/tree/f6440be7b8,
//   numpy/random/src/distributions/distributions.c.
// Copyright (c) NumPy Developers.
// Licensed under the BSD-3-Clause license; see LICENSE.txt.
//
// Macros for randompack compatibility:
#define bitgen_t randompack_rng
#define next_uint64 draw_u64
#define next_double(rng) ((draw_u64(rng) >> 11) * 0x1.0p-53)
//#define next_double randompack_u01_draw
#define npy_log1p log1p

double random_standard_exponential(bitgen_t *bitgen_state);

static double standard_exponential_unlikely(bitgen_t *bitgen_state,
                                                uint8_t idx, double x) {
  if (idx == 0) {
    /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
    return ziggurat_exp_r - npy_log1p(-next_double(bitgen_state));
  } else if ((fe_double[idx - 1] - fe_double[idx]) * next_double(bitgen_state) +
                 fe_double[idx] <
             exp(-x)) {
    return x;
  } else {
    return random_standard_exponential(bitgen_state);
  }
}

double random_standard_exponential(bitgen_t *bitgen_state) {
  uint64_t ri;
  uint8_t idx;
  double x;
  ri = next_uint64(bitgen_state);
  ri >>= 3;
  idx = ri & 0xFF;
  ri >>= 8;
  x = ri * we_double[idx];
  if (ri < ke_double[idx]) {
    return x; /* 98.9% of the time we return here 1st try */
  }
  return standard_exponential_unlikely(bitgen_state, idx, x);
}
