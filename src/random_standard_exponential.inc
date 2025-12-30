// -*- C -*-
// Code copied from NumPy: github.com/numpy/numpy/tree/f6440be7b8,
//   numpy/random/src/distributions/distributions.c.
// Copyright (c) NumPy Developers.
// Licensed under the BSD-3-Clause license; see LICENSE.txt.
//
// Macros for randompack compatibility:
#define bitgen_t randompack_rng
#define next_uint32 draw_u32
#define next_uint64 draw_u64
#define next_float(rng) U32_TO_FLOAT(draw_u32(rng))
#define next_double(rng) U64_TO_DOUBLE(draw_u64(rng))
#define npy_log1p log1p
#define npy_log1pf log1pf

// static inline double random_standard_exponential(bitgen_t *bitgen_state);
// static inline float random_standard_exponential_f(bitgen_t *bitgen_state);
static inline void rand_exp(double *x, size_t len, bitgen_t *bitgen_state);
static inline void rand_expf(float *x, size_t len, bitgen_t *bitgen_state);

// Following four functions (including the two commented out ones) are copied verbatim
// from NumPy, except for marked changes


static inline double standard_exponential_unlikely(bitgen_t *bitgen_state,
                                                    uint8_t idx, double x) {
  while (true) { // added loop to replace recursion
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

static inline float standard_exponential_unlikely_f(bitgen_t *bitgen_state,
                                                    uint8_t idx, float x) {
  while (true) { // added loop to replace recursion
    if (idx == 0) {
      /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
      return ziggurat_exp_r_f - npy_log1pf(-next_float(bitgen_state));
    }
    else if ((fe_float[idx - 1] - fe_float[idx])*next_float(bitgen_state) +
             fe_float[idx] < expf(-x)) {
      return x;
    }
    else {
      // Replaced calling rand_expf with the following:
      uint32_t ri = next_uint32(bitgen_state);
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

// static inline double random_standard_exponential(bitgen_t *bitgen_state) {
//   uint64_t ri;
//   uint8_t idx;
//   double x;
//   ri = next_uint64(bitgen_state);
//   ri >>= 3;
//   idx = ri & 0xFF;
//   ri >>= 8;
//   x = ri * we_double[idx];
//   if (ri < ke_double[idx]) {
//     return x; /* 98.9% of the time we return here 1st try */
//   }
//   return standard_exponential_unlikely(bitgen_state, idx, x);
// }

// Following randompack functions add a loop to fill buf, but otherwise the code is
// unchanged
static inline void rand_exp(double *x, size_t len, bitgen_t *bitgen_state) {
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

static inline void rand_expf(float *x, size_t len, bitgen_t *bitgen_state) {
  for (size_t i=0; i<len; i++) {
	 uint32_t ri;
	 uint8_t idx;
	 float xi;
	 ri = next_uint32(bitgen_state);
	 ri >>= 1;
	 idx = ri & 0xFF;
	 ri >>= 8;
	 xi = ri * we_float[idx];
	 if (ri < ke_float[idx]) { // 98.9% of the time
		x[i] = xi;
	 }
	 else { 
		x[i] = standard_exponential_unlikely_f(bitgen_state, idx, xi);
	 }
  }
}
