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
// hi*y < (xi - x)*(-f'(xi)) <==> y < (2^52 - r)*wi*xi*f(xi)/hi <==> y < dr*slope_norm[i]
//   For xi < 1:     reject if above tangent, accept if below secant
//   For x{i-1} > 1: reject if below tangent, accept if above secant

#define DBL_SIGNBIT 0x8000000000000000ULL

#include <stdint.h>
#include <math.h>
#include <string.h>
#include "printX.h"
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

ALWAYS_INLINE double flip_sign_if(double x, int sign) {
  uint64_t bits, mask = (uint64_t)0 - (uint64_t)(sign & 1);
  mask &= DBL_SIGNBIT;
  memcpy(&bits, &x, sizeof bits);
  bits ^= mask;
  memcpy(&x, &bits, sizeof bits);
  return x;
}

static double norm_tail_draw(randompack_rng *rng, uint64_t rabs, double y, int *w) {
  double xx, yy, x;
  for (;;) {
    /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
    x = draw_u01(rng, w);
    xx = -ziggurat_nor_inv_r * openlibm_log1p(-x);
    yy = -openlibm_log1p(-y);
    if (LIKELY(yy + yy > xx * xx))
      return ((rabs >> 8) & 0x1) ? -(ziggurat_nor_r + xx) : ziggurat_nor_r + xx;
    y = draw_u01(rng, w);
  }
}

ALWAYS_INLINE double norm_likely(uint64_t r, int *idx, uint64_t *ki, uint64_t *rabs) {
  int sign;
  double x;
  *idx = r & 0xff;
  r >>= 8;
  *ki = ki_double[*idx];
  sign = r & 0x1;
  *rabs = (r >> 1) & 0x000fffffffffffff;
  x = (*rabs)*wi_double[*idx];
  x = flip_sign_if(x, sign);
  return x;
}

ALWAYS_INLINE double norm_unlikely(randompack_rng *rng, int *w, double x, int idx,
											  uint64_t *rabs, uint64_t ki) {
  double y, f0, f1, tangent;
  uint64_t r, dr, dk;
  while (true) {
	 y = draw_u01(rng, w);
	 dr = (1ULL << 52) - *rabs;
	 tangent = dr*slope_norm[idx];
	 if (LIKELY(idx > 52)) { // idx == 52 straddles the inflection at x=1
		if (y <= tangent) return x;    // below tangent
		dk = (1ULL << 52) - ki;
		if (y*dk > dr) goto next_try;     // above secant
	 }
	 else if (UNLIKELY(idx == 0)) {      // in tail
		x = norm_tail_draw(rng, *rabs, y, w);
		return x;
	 }
	 else if (LIKELY(idx < 52)) {
		if (y > tangent) goto next_try;   // above tangent
		uint64_t dk = (1ULL << 52) - ki;
		if (y*dk <= dr) return x;      // below secant
	 }
	 f0 = fi_double[idx-1];
	 f1 = fi_double[idx];
	 if (f1 + y*(f0 - f1) < openlibm_exp(-x*x/2)) return x; // below curve
  next_try:
	 r = draw_u64(rng, w);
    x = norm_likely(r, &idx, &ki, rabs);
	 if (*rabs < ki) return x;
  }
}

ALWAYS_INLINE void rand_norm(double *buf, size_t len, randompack_rng *rng) {
  int w = enter_u64_mode(rng);
  size_t i = 0;
  while (i < len) {
    size_t rem = len - i;
    uint64_t rabs, ki;
    double x;
    int idx, take;
    take = rem < BUFSIZE ? rem : BUFSIZE;
    for (int j = 0; j < take; j++) {
		uint64_t r = draw_u64(rng, &w);
      x = norm_likely(r, &idx, &ki, &rabs);
      if (LIKELY(rabs < ki)) {
		  buf[i + j] = x;
		}
		else {
		  int w2 = w;
		  buf[i + j] = norm_unlikely(rng, &w2, x, idx, &rabs, ki);
		  w = w2;
		}
    }
    i += take;
  }
  exit_u64_mode(rng, w);
}

ALWAYS_INLINE double exp_likely(uint64_t r, int *idx, uint64_t *ki, uint64_t *rr) {
  r >>= 3;
  *idx = r & 0xFF;
  r >>= 8;
  *rr = r;
  *ki = ke_double[*idx];
  return r*we_double[*idx];
}

ALWAYS_INLINE double exp_unlikely(randompack_rng *rng, int *w, double x, int idx, uint64_t
											 r, uint64_t ki) {
  double y, f0, f1, tangent;
  uint64_t dr, dk;
  while (true) {
    if (UNLIKELY(idx == 0)) {             // in the tail
      x = ziggurat_exp_r - openlibm_log1p(-draw_u01(rng, w));
      return x;
    }
    y = draw_u01(rng, w);
    dr = (1ULL << 53) - r;
    tangent = dr*slope_exp[idx];
    if (y <= tangent) return x;        // below tangent
    dk = (1ULL << 53) - ki;
    if (y*dk > dr) goto next_try;      // above secant
    f0 = fe_double[idx-1];
    f1 = fe_double[idx];
    if (f1 + y*(f0 - f1) < openlibm_exp(-x)) return x; // below curve
  next_try:
    r = draw_u64(rng, w);
    x = exp_likely(r, &idx, &ki, &r);
    if (LIKELY(r < ki)) return x;
  }
}

ALWAYS_INLINE void rand_exp(double *buf, size_t len, randompack_rng *rng) {
  int idx, w = enter_u64_mode(rng), w2;
  uint64_t r, ki;
  size_t i = 0;
  while (i < len) {
    int take = min(len - i, BUFSIZE);
    for (size_t j = i; LIKELY(j < i + take); j++) {
      r = draw_u64(rng, &w);
      double x = exp_likely(r, &idx, &ki, &r);
      if (LIKELY(r < ki))
        buf[j] = x;
		else {
		  w2 = w;
		  buf[j] = exp_unlikely(rng, &w, x, idx, r, ki);
		  w = w2;
		}
    }
    i += take;
  }
  exit_u64_mode(rng, w);
}

#include "norm_exp_float.inc"
