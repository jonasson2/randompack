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

static double norm_tail_draw(randompack_rng *rng, uint64_t rabs, double y, int *w) {
  double xx, yy, x;
  for (;;) {
    /* Switch to 1.0 - U to avoid log(0.0), see GH 13361 */
    // uint64_t u = draw_u64_nocheck(rng, w);
    x = draw_u01_nocheck(rng, w);
    xx = -ziggurat_nor_inv_r * openlibm_log1p(-x);
    yy = -openlibm_log1p(-y);
    if (LIKELY(yy + yy > xx * xx))
      return ((rabs >> 8) & 0x1) ? -(ziggurat_nor_r + xx) : ziggurat_nor_r + xx;
    y = draw_u01_nocheck(rng, w);
	 make_half_full(rng, w);
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

ALWAYS_INLINE double norm_likely(randompack_rng *rng, int *w, int *idx,
                                     uint64_t *ki, uint64_t *rabs) {
  uint64_t r;
  int sign;
  double x;
  r = draw_u64_nocheck(rng, w);
  *idx = r & 0xff;
  r >>= 8;
  *ki = ki_double[*idx];
  sign = r & 0x1;
  *rabs = (r >> 1) & 0x000fffffffffffff;
  x = (*rabs)*wi_double[*idx];
  x = flip_sign_if(x, sign);
  return x;
}

ALWAYS_INLINE double norm_unlikely(randompack_rng *rng, int *w, double x,
											  int idx, uint64_t *rabs, uint64_t ki) {
  double y, f0, f1, tangent;
  uint64_t dr, dk;
  for(;;) {
	 y = draw_u01_nocheck(rng, w);
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
	 x = norm_likely(rng, w, &idx, &ki, rabs);
	 if (*rabs < ki) return x;
	 make_half_full(rng, w);
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
    make_half_full(rng, &w);
    take = rem < BUFSIZE/2 ? rem : BUFSIZE/2;
    for (int j=0; j<take; j++) {
      x = norm_likely(rng, &w, &idx, &ki, &rabs);
      if (LIKELY(rabs < ki)) {
		  buf[i + j] = x;
		  continue;
		}
		buf[i + j] = norm_unlikely(rng, &w, x, idx, &rabs, ki);
		make_half_full(rng, &w);
    }
    i += take;
  }
  exit_u64_mode(rng, w);
}

ALWAYS_INLINE double draw_exp(randompack_rng *rng, int *w) {
  double x;
  for (;;) {
    double f0, f1;
    uint64_t r = draw_u64_nocheck(rng, w);
    r >>= 3;
    uint8_t idx = r & 0xFF;
    r >>= 8;
    uint64_t ki = ke_double[idx];
    x = r*we_double[idx];;
    if (LIKELY(r < ki)) return x;         // in the left rectangle
    if (UNLIKELY(idx == 0)) {             // in the tail
      x = ziggurat_exp_r - openlibm_log1p(-draw_u01_nocheck(rng, w));
      return x;
    }
    double y = draw_u01_nocheck(rng, w);
    uint64_t dr = (1ULL << 53) - r;
    double tangent = dr*slope_exp[idx];
    if (y <= tangent) return x;        // below tangent
    uint64_t dk = (1ULL << 53) - ki;
    if (y*dk > dr) goto next_try;         // above secant
    f0 = fe_double[idx-1];
    f1 = fe_double[idx];
    if (f1 + y*(f0 - f1) < openlibm_exp(-x)) return x; // below curve
  next_try:
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
