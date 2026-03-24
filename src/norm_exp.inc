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

#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64)
#define TOP_BYTE(dst, src) memcpy(&(dst), (uint8_t*)(&(src)) + 7, 1)
#else
#define TOP_BYTE(dst, src) ((dst) = (uint8_t)((src) >> 56))
#endif

#define NORM_LIKELY_DECODE(x, r, idx, ki, rabs)                         \
 ki = ki_double[idx];                                                   \
 rabs = r & 0x000fffffffffffff;                                         \
 x = ((r >> 52) & 1 ? -(int64_t)rabs : (int64_t)rabs) * wi_double[idx];

#include <stdint.h>
#include <math.h>
#include <string.h>
#include "printX.h"
#include "randompack.h"
#include "ziggurat_constants.h"
#include "zig_const.h"
#include "assert.h"

static uint64_t norm_pdf_calls = 0;
static uint64_t norm_draws = 0;
static uint64_t norm_slow_tail = 0;
static uint64_t norm_slow_accept = 0;
static uint64_t norm_slow_retry = 0;
static uint64_t norm_slow_pdf_accept = 0;
static uint64_t norm_slow_pdf_reject = 0;

void randompack_norm_counters_reset(void) {
  norm_pdf_calls = 0;
  norm_draws = 0;
  norm_slow_tail = 0;
  norm_slow_accept = 0;
  norm_slow_retry = 0;
  norm_slow_pdf_accept = 0;
  norm_slow_pdf_reject = 0;
}

void randompack_norm_counters_get(uint64_t *draws, uint64_t *pdf_calls) {
  if (draws) *draws = norm_draws;
  if (pdf_calls) *pdf_calls = norm_pdf_calls;
}

void randompack_norm_counters_get_slow(uint64_t *tail, uint64_t *accept,
  uint64_t *retry, uint64_t *pdf_accept, uint64_t *pdf_reject) {
  if (tail) *tail = norm_slow_tail;
  if (accept) *accept = norm_slow_accept;
  if (retry) *retry = norm_slow_retry;
  if (pdf_accept) *pdf_accept = norm_slow_pdf_accept;
  if (pdf_reject) *pdf_reject = norm_slow_pdf_reject;
}

//=============================== NORMAL DISTRIBUTION ==============================

NEVER_INLINE double norm_tail_draw(randompack_rng *rng, uint64_t rabs, double y, int *w){
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

NEVER_INLINE bool below_normpdf(uint8_t idx, double x, double y) {
  double f0 = fi_double[idx-1];
  double f1 = fi_double[idx];
  return f1 + y*(f0 - f1) < openlibm_exp(-x*x/2);
}

#define DBL(x) ((x)*0x1.0p-52)

ALWAYS_INLINE double norm_unlikely(randompack_rng *rng, int *w, double x,
                                   uint8_t idx, uint64_t rabs, uint64_t ki) {
  printSetNdec(6);
  uint64_t ru, tmp, L, R, Y, YL;
  const int inflection_idx = 52;
  double f0, f1, y;
  for (;;) {
    Y = draw_u64(rng, w);
    L = (1ULL << 52) - ki;
    R = (1ULL << 52) - rabs;
    double u = U64_TO_DOUBLE(Y);
    if (UNLIKELY(idx == 0)) {   //
      x = norm_tail_draw(rng, rabs, u, w);
      return x;
    }
    MUL64_WIDE(Y, L, YL, tmp);
	 if (YL + Dzig2[idx] < R) return x;
	 if (YL > R + Dzig1[idx]) goto next_try;
	 // if (LIKELY(idx > inflection_idx)) {
    //   if (YL > R) goto next_try;
    //   if (YL + Dzig[idx] < R) return x;
    // }
    // else if (LIKELY(idx < inflection_idx)) {
    //   if (YL < R) return x;
    //   if (YL > R + Dzig[idx]) goto next_try;
    // }
    // else {
    //   if (YL > R + Dzig52) goto next_try;
    //   if (YL + Dzig[idx] < R) return x;
    // }
    f0 = fi_double[idx-1];
    f1 = fi_double[idx];
    y = f1 + u*(f0 - f1);
    if (below_normpdf(idx, x, y)) return x;
  next_try:
    ru = draw_u64(rng, w);
    TOP_BYTE(idx, ru);
    NORM_LIKELY_DECODE(x, ru, idx, ki, rabs);
    if (LIKELY(rabs < ki)) return x;
  }
}

ALWAYS_INLINE double draw_norm(randompack_rng *rng, int *w) {
  uint64_t rabs, ki, r;
  double x;
  uint8_t idx;
  r = draw_u64(rng, w);
  TOP_BYTE(idx, r);
  NORM_LIKELY_DECODE(x, r, idx, ki, rabs);
  if (LIKELY(rabs < ki)) return x;
  x = norm_unlikely(rng, w, x, idx, rabs, ki);
  return x;
}

NEVER_INLINE void rand_norm(double *buf, size_t len, randompack_rng *rng) {
  align64(rng);
  draw_raw_copy((uint8_t *)buf, len*sizeof(uint64_t), rng);
  int w = enter_u64_mode(rng);
  uint8_t idx;
  uint64_t r, rabs, ki;
  double x;
  for (size_t i = len; i-- > 0;) {
    uint8_t *bufpi = (uint8_t*)(buf + i);
    memcpy(&r, bufpi, sizeof(r));
#if defined(__x86_64__) || defined(__amd64__) || defined(_M_X64)
    TOP_BYTE(idx, buf[i]);
#else
    TOP_BYTE(idx, r);
#endif
    NORM_LIKELY_DECODE(x, r, idx, ki, rabs);
    buf[i] = LIKELY(rabs < ki) ? x : norm_unlikely(rng, &w, x, idx, rabs, ki);
  }
  exit_u64_mode(rng, w);
}

//========================= exponential distribution ===========================

NEVER_INLINE bool below_exppdf(uint8_t idx, double x, double y) {
  // Use openlibm_exp to ensure cross-platform compatibility (with exp we
  // the result of the test might depend on the platform)
  double f0, f1;
  f0 = fe_double[idx-1];
  f1 = fe_double[idx];
  return (f1 + y*(f0 - f1) < openlibm_exp(-x)); // below curve?
}

ALWAYS_INLINE double exp_unlikely(randompack_rng *rng, int *w, double x, uint8_t idx,
                                      uint64_t r) {
  double y, tangent;
  uint64_t ke = ke_double[idx], dr, dk;
  while (true) {
    if (UNLIKELY(idx == 0)) {             // in the tail
      x = ziggurat_exp_r - openlibm_log1p(-draw_u01(rng, w));
      return x;
    }
    y = draw_u01(rng, w);
    dr = (1ULL << 53) - r;
    tangent = dr*slope_exp[idx];
    if (y <= tangent) return x;        // below tangent
    dk = (1ULL << 53) - ke;
    if (y*dk > dr) goto next_try;      // above secant
    if (below_exppdf(idx, x, y)) return x; // below curve
  next_try:
    r = draw_u64(rng, w);
    TOP_BYTE(idx, r);
    r &= 0x1FFFFFFFFFFFFF;
    ke = ke_double[idx];
    x = r*we_double[idx];
    if (LIKELY(r < ke)) return x;
  }
}

ALWAYS_INLINE void rand_exp(double *buf, size_t len, randompack_rng *rng) {
  align64(rng);
  draw_raw_copy((uint8_t *)buf, len*sizeof(uint64_t), rng);
  int w = enter_u64_mode(rng);
  uint8_t idx;
  uint64_t r;
  double x, *bufi = buf;
  for (size_t i = len; i > 0; i--) {
    memcpy(&r, bufi, sizeof(r));
    TOP_BYTE(idx, *(uint64_t*)bufi);
    r &= 0x1FFFFFFFFFFFFF;
    x = r*we_double[idx];
    *bufi++ = LIKELY(r < ke_double[idx]) ? x : exp_unlikely(rng, &w, x, idx, r);
  }
  exit_u64_mode(rng, w);
}

#include "norm_exp_float.inc"
