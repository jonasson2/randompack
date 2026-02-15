// -*- C -*-
// Code copied from NumPy: github.com/numpy/numpy/tree/f6440be7b8,
//   numpy/random/src/distributions/distributions.c.
// and adapted to randompack. No change to computations.
// Copyright (c) NumPy Developers.
//
// Strips are numbered from top to bottom: 1, 2,...,255, 0
// Bottom right corner of strip i is at (xi, fi).
// Accept if x <= x{i-1}.
// Draw y if x{i-1} < x <= x{i}
//
// Tangent for exp:
// x = r*wi where r is a 53 bit random
// xi = 2^53*wi
// hi = f{i-1} - fi
// hi*y < (xi - x)*(-f'(xi)) <==> y < (2^53 - r)*wi*f(xi)/hi <==> y < dr*slope_exp[i]
//
// Tangent for normals:
// x = r*wi were r is a 52 bit normal
// xi = 2^52*wi
// hi*y < (xi - x)*(-f'(xi)) <==> y < (2^53 - r)*wi*xi*f(xi)/hi <==> y < dr*slope_norm[i]
//   For xi < 1:     reject if above tangent, accept if below secant
//   For x{i-1} > 1: reject if below tangent, accept if above secant

#define DBL_SIGNBIT 0x8000000000000000ULL

#include <stdint.h>
#include <math.h>
#include <string.h>
#include "randompack.h"
#include "ziggurat_constants.h"
#undef NDEBUG
#include "assert.h"

static int countAboveSecant = 0;
static int countBelowTangent = 0;
static int countAboveCurve = 0;
static int countBelowCurve = 0;
static int countTail = 0;

void randompack_get_counters(int *cas, int *cbt, int *cc, int *cbc, int *ctail) {
  *cas = countAboveSecant;
  *cbt = countBelowTangent;
  *cc = countAboveCurve;
  *cbc = countBelowCurve;
  *ctail = countTail;
}

static double norm_tail_draw(randompack_rng *rng, uint64_t rabs, int *w) {
  double xx, yy;
  for (;;) {
    /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
    xx = -ziggurat_nor_inv_r * openlibm_log1p(-draw_u01(rng, w));
    yy = -openlibm_log1p(-draw_u01(rng, w));
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
    uint64_t ki = ke_double[idx];
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
    double y = draw_u01(rng, w);
    uint64_t dk = (1ULL << 53) - ki;
    uint64_t dr = (1ULL << 53) - r;
    if (UNLIKELY(idx == 52)) {
      double f0 = fe_double[idx-1], f1 = fe_double[idx];
      if (f1 + y*(f0 - f1) < openlibm_exp(-x*x/2)) { // below curve
        make_half_full(rng, w);
        return x;
      }
    }
	 else if (idx < 52) {
      if (y*dk <= dr) { // below secant
        make_half_full(rng, w);
        return x;
      }
      if (y <= dr*slope_norm[idx]) { // below tangent
        double f0 = fe_double[idx-1], f1 = fe_double[idx];
        if (f1 + y*(f0 - f1) < openlibm_exp(-x*x/2)) { // below curve
          make_half_full(rng, w);
          return x;
        }
      }
    }
    else {
      if (y <= dr*slope_norm[idx]) { // below tangent
        make_half_full(rng, w);
        return x;
      }
      if (y*dk <= dr) { // below secant
        double f0 = fe_double[idx-1], f1 = fe_double[idx];
        if (f1 + y*(f0 - f1) < openlibm_exp(-x*x/2)) { // below curve
          make_half_full(rng, w);
          return x;
        }
      }
    }      
    // if (h*y + fi_double[idx] < openlibm_exp(-0.5*x*x)) {
    //   make_half_full(rng, w);
    //   return x;
    // }
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

// Following four functions are based on NumPy
ALWAYS_INLINE double standard_exponential_unlikely(randompack_rng *rng,
  uint8_t idx, uint64_t ki, double wi, uint64_t r, double x, int *w) {
  while (true) { // added loop to replace recursion, to allow speedup.
    if (UNLIKELY(idx == 0)) {
      /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
      return ziggurat_exp_r - openlibm_log1p(-draw_u01(rng, w));
    }
    else {
      double y = draw_u01(rng, w);
		uint64_t r0 = (1ULL << 53) - ki;
		uint64_t r1 = r - ki;
		if (r0*y > r1) {
		  double f1 = fe_double[idx-1], f0 = fe_double[idx];
		  double yh = y*(f1 - f0);
		  if (yh >= r1*wi*f1) return x;
		  if (UNLIKELY(f1 - yh < openlibm_exp(-x))) return x;
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
  // double shift = 0;
  double x;
  while (true) {
    uint64_t r = draw_u64_nocheck(rng, w);
    r >>= 3;
    uint8_t idx = r & 0xFF;
    r >>= 8;
    uint64_t ki = ke_double[idx];
    x = r*we_double[idx];;
    if (LIKELY(r < ki)) return x;
    if (UNLIKELY(idx == 0)) { // 0.045%
      x = ziggurat_exp_r + openlibm_log1p(-draw_u01(rng, w));
      make_half_full(rng, w);
      return x;
    }
    double y = draw_u01(rng, w);
    uint64_t dr = (1ULL << 53) - r;
    if (y <= dr*slope_exp[idx]) { // below tangent (1.07%)
      make_half_full(rng, w);
      return x;
    }
    uint64_t dk = (1ULL << 53) - ki;
    if (y*dk <= dr) { // below secant (0.030%)
      double f0 = fe_double[idx-1], f1 = fe_double[idx];
      if (f1 + y*(f0 - f1) < openlibm_exp(-x)) { // below curve (0.020%)
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
