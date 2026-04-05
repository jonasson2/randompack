// -*- C -*-
// Alternative normal generator using the Marsaglia polar method.
// The exponential generator below is kept identical to norm_exp.inc.

#include <stdint.h>
#include <math.h>
#include <string.h>
#include "randompack.h"
#include "ziggurat_const_double.h"
#include "zig_D.h"

//=============================== NORMAL DISTRIBUTION ==============================

ALWAYS_INLINE double polar_scale(double s) {
  return sqrt(-2*log(s)/s);
}

NEVER_INLINE void polar_pair(randompack_rng *rng, int *w, double *x, double *y) {
  double u, v, s, m;
  do {
    u = 2*draw_u01(rng, w) - 1;
    v = 2*draw_u01(rng, w) - 1;
    s = u*u + v*v;
  } while (s >= 1 || s == 0);
  m = polar_scale(s);
  *x = u*m;
  *y = v*m;
}

ALWAYS_INLINE double draw_norm(randompack_rng *rng, int *w) {
  double x, y;
  polar_pair(rng, w, &x, &y);
  return x;
}

NEVER_INLINE void rand_norm(double *buf, size_t len, randompack_rng *rng) {
  int w = enter_u64_mode(rng);
  size_t i = 0;
  while (i + 1 < len) {
    double x, y;
    polar_pair(rng, &w, &x, &y);
    buf[i++] = x;
    buf[i++] = y;
  }
  if (i < len) {
    double x, y;
    polar_pair(rng, &w, &x, &y);
    buf[i] = x;
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

NEVER_INLINE double exp_taildraw(randompack_rng *rng, int *w) {
  double x = ziggurat_exp_r - openlibm_log1p(-draw_u01(rng, w));
  return x;
}

ALWAYS_INLINE double exp_unlikely(randompack_rng *rng, int *w, double x, uint8_t idx,
  uint64_t r) {
  double y;
  uint64_t Y, YL, R, L, tmp, ke = ke_double[idx];
  while (true) {
    if (UNLIKELY(idx == 0)) return exp_taildraw(rng, w);
    Y = draw_u64(rng, w);
    L = (1ULL << 53) - ke;
    R = (1ULL << 53) - r;
    MUL64_WIDE(Y, L, YL, tmp);
    (void)tmp;
    if (LIKELY(YL > R)) goto next_try;         // above secant
    if (YL + Dexp[idx] < R) return x;  // below pdf by the gap bound
    y = U64_TO_DOUBLE(Y);
    if (below_exppdf(idx, x, y)) return x; // below curve
  next_try:
    r = draw_u64(rng, w);
    idx = r >> 56;
    r &= 0x1FFFFFFFFFFFFF;
    ke = ke_double[idx];
    x = r*we_double[idx];
    if (LIKELY(r < ke)) return x;
  }
}

NEVER_INLINE void rand_exp(double *buf, size_t len, randompack_rng *rng) {
  align64(rng);
  draw_raw_copy((uint8_t *)buf, len*sizeof(uint64_t), rng);
  int w = enter_u64_mode(rng);
  uint8_t idx;
  uint64_t r;
  double x;
  for (size_t i = len; i-- > 0;) {
    uint8_t *bufpi = (uint8_t*)(buf + i);
    memcpy(&r, bufpi, sizeof(r));
    memcpy(&idx, (uint8_t*)(buf + i) + 7, 1);  // measured slightly faster than >> 56
    r &= 0x1FFFFFFFFFFFFF;
    x = r*we_double[idx];
    buf[i] = LIKELY(r < ke_double[idx]) ? x : exp_unlikely(rng, &w, x, idx, r);
  }
  exit_u64_mode(rng, w);
}
