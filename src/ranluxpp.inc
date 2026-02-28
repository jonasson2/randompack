// -*- C -*-
static void
mul9x9(uint64_t *z, const uint64_t *x, const uint64_t *y) {
  uint64_t carry_lo = 0;
  uint64_t carry_hi = 0;
  for (int k = 0; k < 18; k++) {
    uint64_t acc0 = carry_lo;
    uint64_t acc1 = carry_hi;
    uint64_t acc2 = 0;
    for (int i = 0; i < 9; i++) {
      int j = k - i;
      if (j < 0 || j >= 9) continue;
      uint64_t hi, lo;
      MUL64_WIDE(x[i], y[j], hi, lo);
      uint64_t t0 = acc0 + lo;
      uint64_t c0 = (t0 < acc0);
      acc0 = t0;
      uint64_t t1 = acc1 + hi;
      uint64_t c1 = (t1 < acc1);
      acc1 = t1;
      uint64_t t2 = acc1 + c0;
      uint64_t c2 = (t2 < acc1);
      acc1 = t2;
      acc2 += c1 + c2;
    }
    z[k] = acc0;
    carry_lo = acc1;
    carry_hi = acc2;
  }
}

static uint64_t
add576(uint64_t *r, const uint64_t *a) {
  uint64_t carry = 0;
  for (int i = 0; i < 9; i++) {
    uint64_t x = r[i];
    uint64_t y = a[i];
    uint64_t s = x + y;
    uint64_t c1 = (s < x);
    uint64_t s2 = s + carry;
    uint64_t c2 = (s2 < s);
    r[i] = s2;
    carry = c1|c2;
  }
  return carry;
}

static uint64_t
sub576(uint64_t *r, const uint64_t *a) {
  uint64_t borrow = 0;
  for (int i = 0; i < 9; i++) {
    uint64_t x = r[i];
    uint64_t y = a[i];
    uint64_t t = x - y;
    uint64_t b1 = (t > x);
    uint64_t t2 = t - borrow;
    uint64_t b2 = (t2 > t);
    r[i] = t2;
    borrow = b1|b2;
  }
  return borrow;
}

static int
cmp576(const uint64_t *a, const uint64_t *b) {
  for (int i = 8; i >= 0; i--) {
    if (a[i] > b[i]) return 1;
    if (a[i] < b[i]) return -1;
  }
  return 0;
}

static void
add_small(uint64_t *r, uint64_t k) {
  uint64_t carry = k;
  for (int i = 0; i < 9 && carry != 0; i++) {
    uint64_t x = r[i];
    r[i] = x + carry;
    carry = (r[i] < x);
  }
}

static void
sub_small(uint64_t *r, uint64_t k) {
  uint64_t borrow = k;
  for (int i = 0; i < 9 && borrow != 0; i++) {
    uint64_t x = r[i];
    r[i] = x - borrow;
    borrow = (r[i] > x);
  }
}

static void
add_shift240_small(uint64_t *r, uint64_t k) {
  if (k == 0) return;
  uint64_t lo = k<<48;
  uint64_t hi = k>>16;
  uint64_t x3 = r[3];
  r[3] = x3 + lo;
  uint64_t carry = (r[3] < x3);
  uint64_t add4 = hi + carry;
  if (add4 != 0) {
    uint64_t x4 = r[4];
    r[4] = x4 + add4;
    carry = (r[4] < x4);
    for (int i = 5; i < 9 && carry != 0; i++) {
      uint64_t xi = r[i];
      r[i] = xi + 1;
      carry = (r[i] < xi);
    }
  }
}

static void
sub_shift240_small(uint64_t *r, uint64_t k) {
  if (k == 0) return;
  uint64_t lo = k<<48;
  uint64_t hi = k>>16;
  uint64_t x3 = r[3];
  r[3] = x3 - lo;
  uint64_t borrow = (r[3] > x3);
  uint64_t sub4 = hi + borrow;
  if (sub4 != 0) {
    uint64_t x4 = r[4];
    r[4] = x4 - sub4;
    borrow = (r[4] > x4);
    for (int i = 5; i < 9 && borrow != 0; i++) {
      uint64_t xi = r[i];
      r[i] = xi - 1;
      borrow = (r[i] > xi);
    }
  }
}

static void
shift240_left(uint64_t *dst, const uint64_t *src) {
  for (int i = 0; i < 9; i++) dst[i] = 0;
  for (int i = 3; i < 9; i++) {
    uint64_t a = src[i-3];
    uint64_t b = (i >= 4) ? src[i-4] : 0;
    dst[i] = (a<<48) | (b>>16);
  }
}

static void
mul9x9_remainder(uint64_t *r, const uint64_t *z) {
  // m = 2^576 - 2^240 + 1, little-endian u64[9]
  static const uint64_t MOD_M[9] = {
    0x0000000000000001ULL,
    0x0000000000000000ULL,
    0x0000000000000000ULL,
    0xffff000000000000ULL,
    0xffffffffffffffffULL,
    0xffffffffffffffffULL,
    0xffffffffffffffffULL,
    0xffffffffffffffffULL,
    0xffffffffffffffffULL
  };
  // z[18] = [t0 | t1] where t0 = z[0..8], t1 = z[9..17]
  // m = 2^576 - 2^240 + 1
  // Since 2^576 ≡ 2^240 - 1 (mod m):
  //   z = t0 + t1*2^576 ≡ t0 - t1 + t1*2^240 (mod m).
  // Let t1 = t3 + t2*2^336 where:
  //   t2 = upper 240 bits of t1 (t1 bits 336..575), packed at bit 0
  //   t3 = lower 336 bits of t1 (t1 bits 0..335)
  // Then t1*2^240 = t3*2^240 + t2*2^576 ≡ (t3+t2)*2^240 - t2 (mod m),
  // so:
  //   r ≡ t0 - t1 - t2 + (t3+t2)*2^240 (mod m).
  const uint64_t *t0 = z;
  const uint64_t *t1 = z + 9;
  uint64_t t2[9] = {0};
  uint64_t t3[9] = {0};
  uint64_t u[9];
  uint64_t v[9];
  t2[0] = (t1[5]>>16) | (t1[6]<<48);
  t2[1] = (t1[6]>>16) | (t1[7]<<48);
  t2[2] = (t1[7]>>16) | (t1[8]<<48);
  t2[3] = (t1[8]>>16);
  t3[0] = t1[0];
  t3[1] = t1[1];
  t3[2] = t1[2];
  t3[3] = t1[3];
  t3[4] = t1[4];
  t3[5] = t1[5] & 0xffffULL;
  for (int i = 0; i < 9; i++) u[i] = t3[i];
  add576(u, t2);
  shift240_left(v, u);
  for (int i = 0; i < 9; i++) r[i] = t0[i];
  int64_t c = 0;
  c -= (int64_t)sub576(r, t1);
  c -= (int64_t)sub576(r, t2);
  c += (int64_t)add576(r, v);
  if (c > 0) {
    add_shift240_small(r, (uint64_t)c);
    sub_small(r, (uint64_t)c);
  }
  else if (c < 0) {
    uint64_t k = (uint64_t)(-c);
    sub_shift240_small(r, k);
    add_small(r, k);
  }
  if (cmp576(r, MOD_M) >= 0) sub576(r, MOD_M);
}

static void
mul9x9mod(uint64_t *b, const uint64_t *a) {
  uint64_t z[18];
  mul9x9(z, a, b);
  mul9x9_remainder(b, z);
}

static inline void fill_ranluxpp(randompack_rng *rng, size_t len) {
  static const uint64_t A[9] = {
    0xed7faa90747aaad9UL,  // word 0
    0x4cec2c78af55c101UL,  // word 1
    0xe64dcb31c48228ecUL,  // word 2
    0x6d8a15a13bee7cb0UL,  // word 3
    0x20b2ca60cb78c509UL,  // word 4
    0x256c3d3c662ea36cUL,  // word 5
    0xff74e54107684ed2UL,  // word 6
    0x492edfcc0cc8e753UL,  // word 7
    0xb48c187cf5b22097UL,  // word 8
  };
  uint64_t *out = rng->buf.u64;
  uint64_t s[9];
  for (int i = 0; i < 9; i++) s[i] = rng->state.u64[i];
  size_t i = 0;
  while (i < len) {
    mul9x9mod(s, A);
    for (int j = 0; j < 9 && i < len; j++, i++)
      out[i] = s[j];
  }
  for (int i = 0; i < 9; i++) rng->state.u64[i] = s[i];
}
