#include "mul9x9.h"

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

void mul9x9(uint64_t *out, const uint64_t *restrict a,
  const uint64_t *restrict b) {
  uint64_t lo, c, cy;
  __uint128_t p, bi;
  for (int i=0; i<18; i++) out[i] = 0;
  for (int i=0; i<9; i++) {
    c = 0;
    cy = 0;
    bi = (__uint128_t)b[i];
    for (int j=0; j<9; j++) {
      p = bi * a[j];
      p += c;
      c = (uint64_t)(p >> 64);
      lo = (uint64_t)p;
      out[i+j] = __builtin_addcll(out[i+j], lo, cy, &cy);
    }
    out[i+9] = c + cy;
  }
}

// Remainder algorithm:
// t0 = x[0..8]
// t1 = x[9..17]
// t2 = upper 240 bits of t1
// t3 = lower 336 bits of t1
// r = t0 - (t1 + t2) + (t3 + t2) * 2^240
// c = floor(r / 2^576)
// r = r - c * m
void mod9x9(uint64_t *x) {
  uint64_t t1[9], t2[9], t3[9], u[9], v[9], r[9];
  uint64_t w5, w6, w7, w8, addv;
  unsigned long long carry, borrow;
  for (int i = 0; i < 9; i++) t1[i] = x[i+9];
  w5 = t1[5];
  w6 = t1[6];
  w7 = t1[7];
  w8 = t1[8];
  t2[0] = (w5 >> 16) | (w6 << 48);
  t2[1] = (w6 >> 16) | (w7 << 48);
  t2[2] = (w7 >> 16) | (w8 << 48);
  t2[3] = (w8 >> 16);
  for (int i = 4; i < 9; i++) t2[i] = 0;
  for (int i = 0; i < 5; i++) t3[i] = t1[i];
  t3[5] = t1[5] & 0xffff;
  for (int i = 6; i < 9; i++) t3[i] = 0;
  carry = 0;
  for (int i = 0; i < 9; i++)
    u[i] = __builtin_addcll(t1[i], t2[i], carry, &carry);
  carry = 0;
  for (int i = 0; i < 9; i++)
    v[i] = __builtin_addcll(t3[i], t2[i], carry, &carry);
  borrow = 0;
  for (int i = 0; i < 9; i++)
    r[i] = __builtin_subcll(x[i], u[i], borrow, &borrow);
  carry = 0;
  for (int i = 0; i < 9; i++) {
    if (i < 3) addv = 0;
    else {
      addv = v[i-3] << 48;
      if (i >= 4) addv |= v[i-4] >> 16;
    }
    r[i] = __builtin_addcll(r[i], addv, carry, &carry);
  }
  int c = (int)carry - (int)borrow;
  if (c > 0) {
    unsigned long long b = 0;
    r[0] = __builtin_subcll(r[0], (uint64_t)c, 0, &b);
    for (int i = 1; i < 9; i++)
      r[i] = __builtin_subcll(r[i], 0, b, &b);
    unsigned long long c0 = 0;
    uint64_t add = ((uint64_t)c) << 48;
    r[3] = __builtin_addcll(r[3], add, 0, &c0);
    for (int i = 4; i < 9; i++)
      r[i] = __builtin_addcll(r[i], 0, c0, &c0);
  }
  else if (c < 0) {
    int n = -c;
    unsigned long long c0 = 0;
    r[0] = __builtin_addcll(r[0], (uint64_t)n, 0, &c0);
    for (int i = 1; i < 9; i++)
      r[i] = __builtin_addcll(r[i], 0, c0, &c0);
    unsigned long long b = 0;
    uint64_t sub = ((uint64_t)n) << 48;
    r[3] = __builtin_subcll(r[3], sub, 0, &b);
    for (int i = 4; i < 9; i++)
      r[i] = __builtin_subcll(r[i], 0, b, &b);
  }
  for (int i = 0; i < 9; i++) x[i] = r[i];
}
