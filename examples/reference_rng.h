// -*- C -*-
// Reference xoshiro256++ helpers for timing and benchmarks.

#ifndef REFERENCE_RNG_H
#define REFERENCE_RNG_H

#include <math.h>
#include <stdint.h>

static inline uint64_t rotl64(uint64_t x, int k) {
  return (x << k) | (x >> (64 - k));
}

static inline uint64_t x256pp_next(uint64_t *s0, uint64_t *s1, uint64_t *s2,
  uint64_t *s3) {
  uint64_t result = rotl64(*s0 + *s3, 23) + *s0;
  uint64_t t = *s1 << 17;
  *s2 ^= *s0;
  *s3 ^= *s1;
  *s1 ^= *s2;
  *s0 ^= *s3;
  *s2 ^= t;
  *s3 = rotl64(*s3, 45);
  return result;
}

static inline double x256pp_u01(uint64_t *s0, uint64_t *s1, uint64_t *s2, uint64_t *s3) {
  uint64_t result = x256pp_next(s0, s1, s2, s3);
  return (double)(result >> 11) * 0x1.0p-53;
}

static inline double x256pp_exponential(uint64_t *s0, uint64_t *s1, uint64_t *s2, uint64_t
													 *s3) {
  double u = x256pp_u01(s0, s1, s2, s3);
  return -log1p(-u);
}

static inline void fill_x256pp_u64(uint64_t *buf, int n, uint64_t *s0, uint64_t *s1,
											  uint64_t *s2, uint64_t *s3) {
  for (int i = 0; i < n; i++) {
    uint64_t v = x256pp_next(s0, s1, s2, s3);
    buf[i] = v;
  }
}

static inline void fill_x256pp_u01(double *buf, int n, uint64_t *s0,
  uint64_t *s1, uint64_t *s2, uint64_t *s3) {
  for (int i = 0; i < n; i++) {
    uint64_t v = x256pp_next(s0, s1, s2, s3);
    buf[i] = (double)(v >> 11) * 0x1.0p-53;
  }
}

static inline void fill_x256pp_exp(double *buf, int n, uint64_t *s0,
  uint64_t *s1, uint64_t *s2, uint64_t *s3) {
  for (int i = 0; i < n; i++) {
    buf[i] = x256pp_exponential(s0, s1, s2, s3);
  }
}

static inline void fill_x256pp_norm_polar(double *buf, int n, uint64_t *s0,
  uint64_t *s1, uint64_t *s2, uint64_t *s3) {
  int i = 0;
  while (i + 1 < n) {
    double u, v, s;
    uint64_t r1, r2;
    do {
      r1 = x256pp_next(s0, s1, s2, s3);
      r2 = x256pp_next(s0, s1, s2, s3);
      // u,v in (-1,1): 2*u01-1 from top 53 bits
      u = 2.0*(double)(r1 >> 11) * 0x1.0p-53 - 1.0;
      v = 2.0*(double)(r2 >> 11) * 0x1.0p-53 - 1.0;
      s = u*u + v*v;
    } while (s >= 1.0 || s == 0.0);
    double m = sqrt((-2.0*log(s))/s);
    buf[i++] = u*m;
    buf[i++] = v*m;
  }
  if (i < n) {
    double u, v, s;
    uint64_t r1, r2;
    do {
      r1 = x256pp_next(s0, s1, s2, s3);
      r2 = x256pp_next(s0, s1, s2, s3);
      u = 2.0*(double)(r1 >> 11) * 0x1.0p-53 - 1.0;
      v = 2.0*(double)(r2 >> 11) * 0x1.0p-53 - 1.0;
      s = u*u + v*v;
    } while (s >= 1.0 || s == 0.0);
    double m = sqrt((-2.0*log(s))/s);
    buf[i] = u*m;
  }
}

#endif
