// -*- C -*-
#include "randompack_config.h"
#include <stdlib.h>
#include <stdbool.h>
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <immintrin.h>
#endif
// let:
//   A = [a0..a8]
//   B = [b0..b8]
//   R = [r0..r17]
// where ai, bi, ri are 64bit integers. Also let
// hi:lo denote a 128 bit integer
// c be 64 bits
// cy be a carry bit
// cout = adc(x, y, cin) be an add-with-carry function that adds y to x

// And the algorithm:
// R = 0
// for i = 0..8:
//   c = 0
//   cy = 0
//   for j = 0..8:
//     hi:lo = b[i]*a[j]
//     hi:lo += c        // 128-bit add
//     c = hi
//     cy = adc(r[i+j], lo, cy)
//   cy = adc(r[i+9], c, cy)

ALWAYS_INLINE uint64_t addc_u64(uint64_t a, uint64_t b, uint64_t cin,
  uint64_t *sum) {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
  unsigned char carry = _addcarry_u64((unsigned char)cin, a, b, sum);
  return (uint64_t)carry;
#else
  uint64_t s;
  bool c0 = __builtin_add_overflow(a, b, &s);
  bool c1 = __builtin_add_overflow(s, cin, &s);
  *sum = s;
  return (uint64_t)(c0 || c1);
#endif
}

ALWAYS_INLINE uint64_t subc_u64(uint64_t a, uint64_t b, uint64_t bin,
  uint64_t *diff) {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
  unsigned char borrow = _subborrow_u64((unsigned char)bin, a, b, diff);
  return (uint64_t)borrow;
#else
  uint64_t d;
  bool b0 = __builtin_sub_overflow(a, b, &d);
  bool b1 = __builtin_sub_overflow(d, bin, &d);
  *diff = d;
  return (uint64_t)(b0 || b1);
#endif
}

ALWAYS_INLINE void mul9x9(uint64_t *z, const uint64_t *restrict x) {
  static const uint64_t a[9] = {
    0xed7faa90747aaad9ULL,
    0x4cec2c78af55c101ULL,
    0xe64dcb31c48228ecULL,
    0x6d8a15a13bee7cb0ULL,
    0x20b2ca60cb78c509ULL,
    0x256c3d3c662ea36cULL,
    0xff74e54107684ed2ULL,
    0x492edfcc0cc8e753ULL,
    0xb48c187cf5b22097ULL,
  };
  uint64_t lo, c, cy;
  __uint128_t p, xi;
  for (int i=0; i<18; i++) z[i] = 0;
  for (int i=0; i<9; i++) {
    c = 0;
    cy = 0;
    xi = (__uint128_t)x[i];
    for (int j=0; j<9; j++) {
      p = xi * a[j];
      p += c;
      c = (uint64_t)(p >> 64);
      lo = (uint64_t)p;
      cy = addc_u64(z[i+j], lo, cy, &z[i+j]);
    }
    z[i+9] = c + cy;
  }
}

// Remainder algorithm:
// [let m = 2^576 - 2^240 + 1]
// t0 = z[0..8]
// t1 = z[9..17]
// t2 = upper 240 bits of t1
// t3 = lower 336 bits of t1
// r = t0 - t1 - t2 + (t3 + t2)*2^240
// c = floor(r/2^576)
// x = r - c*m
// [it is easy to see that 0 <= x < m]

#include <stdint.h>

ALWAYS_INLINE int addbits(uint64_t *x, uint64_t c, int n) {
  uint64_t carry = 0;
  carry = addc_u64(x[0], c, 0, &x[0]);
  for (int i = 1; i < n; i++) {
    carry = addc_u64(x[i], 0, carry, &x[i]);
    if (!carry) break;
  }
  return (int)carry;
}

ALWAYS_INLINE int subbits(uint64_t *x, uint64_t c, int n) {
  uint64_t borrow = 0;
  borrow = subc_u64(x[0], c, 0, &x[0]);
  for (int i = 1; i < n; i++) {
    borrow = subc_u64(x[i], 0, borrow, &x[i]);
    if (!borrow) break;
  }
  return (int)borrow;
}

ALWAYS_INLINE int addc(uint64_t *r, const uint64_t *y, int n) {
  uint64_t carry = 0;
  for (int i = 0; i < n; i++) {
    carry = addc_u64(r[i], y[i], carry, &r[i]);
  }
  return (int)carry;
}

ALWAYS_INLINE int subc(uint64_t *r, const uint64_t *y, int n) {
  uint64_t borrow = 0;
  for (int i = 0; i < n; i++) {
    borrow = subc_u64(r[i], y[i], borrow, &r[i]);
  }
  return (int)borrow;
}

ALWAYS_INLINE void t2get(uint64_t t2[4], uint64_t t1[9]) {
  t2[0] = (t1[5] >> 16) | (t1[6] << 48);
  t2[1] = (t1[6] >> 16) | (t1[7] << 48);
  t2[2] = (t1[7] >> 16) | (t1[8] << 48);
  t2[3] =  t1[8] >> 16;
}

ALWAYS_INLINE int shly240(uint64_t y[9]) {
  int co = (int)((y[5] >> 16) & 1);
  y[8] = (y[5] << 48) | (y[4] >> 16);
  y[7] = (y[4] << 48) | (y[3] >> 16);
  y[6] = (y[3] << 48) | (y[2] >> 16);
  y[5] = (y[2] << 48) | (y[1] >> 16);
  y[4] = (y[1] << 48) | (y[0] >> 16);
  y[3] =  y[0] << 48;
  y[2] = 0;
  y[1] = 0;
  y[0] = 0;
  return co;
}

ALWAYS_INLINE void corr_r_minus_cm(uint64_t r[9], int c) {
  uint64_t
	 c0 = (uint64_t)abs(c),
	 c3 = ((uint64_t)abs(c)) << 48;
  if (c > 0) {
	 subbits(r, c0, 9);
	 addbits(r + 3, c3, 6);
  }
  else if (c < 0) {
	 addbits(r, c0, 9);
	 subbits(r + 3, c3, 6);
  }
}

ALWAYS_INLINE void mod9x9(uint64_t x[9], uint64_t z[18]) {
  uint64_t t1[9], t2[4], t3[9];
  int c;
  copy64(t1, z + 9, 9);
  t2get(t2, t1);
  copy64(t3, z + 9, 6); t3[5] &= 0xFFFF;
  copy64(x, z, 9);
  c = 0;
  c -= subc(x, t1, 9);
  c -= subc(x, t2, 4);
  addc(t3, t2, 4);
  c += shly240(t3);
  c += addc(x, t3, 9);
  corr_r_minus_cm(x, c);
}

ALWAYS_INLINE void fill_ranluxpp(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64;
  uint64_t s[9];
  uint64_t z[18];
  copy64(s, rng->state.u64, 9);
  size_t i = 0;
  while (i < len) {
	 mul9x9(z, s);
	 mod9x9(s, z);
    for (int j = 0; j < 9 && i < len; j++, i++)
      out[i] = s[j];
  }
  copy64(rng->state.u64, s, 9);
}
