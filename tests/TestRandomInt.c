// -*- C -*-
#include <stdbool.h>

#include "randompack.h"
#include "TestRandomHelpers.h"
#include "xCheck.h"

#define RUN_TEST(x)        \
  do {                     \
    xCheckAddMsg(#x);      \
    test_##x();            \
  } while (0)

static void test_int_api(void) {
  const int N = 128;
  int a[N], b[N];
  randompack_rng *r1 = randompack_create("Xorshift", 99);
  randompack_rng *r2 = randompack_create("Xorshift", 99);
  randompack_int(a, N, -3, 8, r1);
  randompack_int(b, N, 0, 11, r2);
  xCheck(min_intv(a, N) >= -3 && max_intv(a, N) <= 8);
  xCheck(min_intv(b, N) >= 0 && max_intv(b, N) <= 11);
  xCheck(equal_intv_offset(a, b, N, 3));
  randompack_free(r1);
  randompack_free(r2);
}

void TestRandomInt(void) {
  RUN_TEST(int_api);
}
