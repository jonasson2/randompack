// -*- C -*-
// Tests for ranluxpp mul9x9.

#include <stdint.h>
#include "randompack.h"
#include "randompack_internal.h"
#include "test_util.h"
#include "xCheck.h"
#define fill_ranluxpp test_fill_ranluxpp
#include "ranluxpp.inc"
#undef fill_ranluxpp

static uint64_t ranlux_a[9] = {
  0xed7faa90747aaad9ULL,
  0x4cec2c78af55c101ULL,
  0xe64dcb31c48228ecULL,
  0x6d8a15a13bee7cb0ULL,
  0x20b2ca60cb78c509ULL,
  0x256c3d3c662ea36cULL,
  0xff74e54107684ed2ULL,
  0x492edfcc0cc8e753ULL,
  0xb48c187cf5b22097ULL
};

void TestMul9x9(void) {
  uint64_t x[9];
  uint64_t z[18];
  uint64_t expect_lo[9];
  uint64_t expect_hi[9];
  // Case 1: x = 0.
  for (int i = 0; i < 9; i++) x[i] = 0;
  mul9x9(z, x);
  CHECK_ZEROV(z, 18);
  // Case 1b: x = 1.
  x[0] = 1;
  for (int i = 1; i < 9; i++) x[i] = 0;
  mul9x9(z, x);
  CHECK_EQUALV(z, ranlux_a, 9);
  CHECK_ZEROV(z + 9, 9);
  // Case 2: x = 2.
  x[0] = 2;
  for (int i = 1; i < 9; i++) x[i] = 0;
  mul9x9(z, x);
  uint64_t carry = 0;
  for (int i = 0; i < 9; i++)
    expect_lo[i] = addc64(ranlux_a[i], ranlux_a[i], carry, &carry);
  expect_hi[0] = carry;
  for (int i = 1; i < 9; i++) expect_hi[i] = 0;
  CHECK_EQUALV(z, expect_lo, 9);
  CHECK_EQUALV(z + 9, expect_hi, 9);
  // Case 3: x = 2^576 - 1 (all UINT64_MAX).
  for (int i = 0; i < 9; i++) x[i] = UINT64_MAX;
  mul9x9(z, x);
  copy64(expect_lo, ranlux_a, 9);
  for (int i = 0; i < 9; i++) expect_lo[i] = ~expect_lo[i];
  expect_lo[0] += 1;
  copy64(expect_hi, ranlux_a, 9);
  expect_hi[0] -= 1;
  CHECK_EQUALV(z, expect_lo, 9);
  CHECK_EQUALV(z + 9, expect_hi, 9);
}

void TestMod9x9(void) {
  uint64_t r[9];
  uint64_t zmod[18];
  uint64_t m[9];
  uint64_t expect[9];
  // Case 1: z = m.
  m[0] = 1;
  m[1] = 0;
  m[2] = 0;
  m[3] = 0xFFFF000000000000ULL;
  m[4] = UINT64_MAX;
  m[5] = UINT64_MAX;
  m[6] = UINT64_MAX;
  m[7] = UINT64_MAX;
  m[8] = UINT64_MAX;
  CLEAR(zmod);
  copy64(zmod, m, 9);
  mod9x9(r, zmod);
  CHECK_EQUALV(r, m, 9);
  // Case 2: z = m + 1.
  CLEAR(zmod);
  copy64(zmod, m, 9);
  zmod[0] += 1;
  mod9x9(r, zmod);
  copy64(expect, m, 9);
  expect[0] += 1;
  CHECK_EQUALV(r, expect, 9);
  // Case 3: z = 2^576.
  CLEAR(zmod);
  zmod[9] = 1;
  mod9x9(r, zmod);
  CLEAR(expect);
  expect[0] = UINT64_MAX;
  expect[1] = UINT64_MAX;
  expect[2] = UINT64_MAX;
  expect[3] = 0x0000FFFFFFFFFFFFULL;
  CHECK_EQUALV(r, expect, 9);
  // Case 4: z = 1.
  CLEAR(zmod);
  zmod[0] = 1;
  mod9x9(r, zmod);
  CLEAR(expect);
  expect[0] = 1;
  CHECK_EQUALV(r, expect, 9);
}

void TestRanluxpp(void) {
  TestMul9x9();
  TestMod9x9();
}
