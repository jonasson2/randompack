#include "mul9x9.h"
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static inline void copy64(void *dst, void *src, int n) { memcpy(dst, src, n*8); }

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

void mul9x9(uint64_t *z, const uint64_t *restrict x) {
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
      z[i+j] = __builtin_addcll(z[i+j], lo, cy, &cy);
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

static inline int addbits(uint64_t *x, uint64_t c, int n) {
  uint64_t carry = 0;
  x[0] = __builtin_addcll(x[0], c, 0, &carry);
  for (int i = 1; i < n; i++) {
	 x[i] = __builtin_addcll(x[i], 0, carry, &carry);
	 if (!carry) break;
  }
  return carry;
}

static inline int subbits(uint64_t *x, uint64_t c, int n) {
  uint64_t borrow = 0;
  x[0] = __builtin_subcll(x[0], c, 0, &borrow);
  for (int i = 1; i < n; i++) {
	 x[i] = __builtin_subcll(x[i], 0, borrow, &borrow);
	 if (!borrow) break;
  }
  return borrow;
}

static inline int addc(uint64_t *r, const uint64_t *y, int n) {
  unsigned long long carry = 0;
  for (int i = 0; i < n; i++) {
    r[i] = __builtin_addcll(r[i], y[i], carry, &carry);
  }
  return (int)carry;
}

static inline int subc(uint64_t *r, const uint64_t *y, int n) {
  unsigned long long borrow = 0;
  for (int i = 0; i < n; i++) {
    r[i] = __builtin_subcll(r[i], y[i], borrow, &borrow);
  }
  return (int)borrow;
}

static inline void t2get(uint64_t t2[4], uint64_t t1[9]) {
  t2[0] = (t1[5] >> 16) | (t1[6] << 48);
  t2[1] = (t1[6] >> 16) | (t1[7] << 48);
  t2[2] = (t1[7] >> 16) | (t1[8] << 48);
  t2[3] =  t1[8] >> 16;
}

static inline int shly240(uint64_t y[9]) {
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

static inline void corr_r_minus_cm(uint64_t r[9], int c) {
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

//static inline void remainder(uint64_t x[9], uint64_t z[18]) {
void mod9x9(uint64_t z[18]) {
  uint64_t t1[9], t2[4], t3[9];
  int c;
  copy64(t1, z + 9, 9);
  t2get(t2, t1);
  copy64(t3, z + 9, 6); t3[5] &= 0xFFFF;
  uint64_t *x = z;
  c = 0;
  c -= subc(x, t1, 9);
  c -= subc(x, t2, 4);
  addc(t3, t2, 4);
  c += shly240(t3);
  c += addc(x, t3, 9);
  corr_r_minus_cm(x, c);
}
