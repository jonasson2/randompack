// -*- C -*-
#include <stdbool.h>

#include "randompack.h"
#include "TestHelpers.h"
#include "xCheck.h"

// 
static void test_int_first(void) {
  const int N = 128;
  int a[N], b[N];
  randompack_rng *r1 = randompack_create("Xorshift", 99);
  randompack_rng *r2 = randompack_create("Xorshift", 99);
  randompack_int(a, N, -3, 8, r1);
  randompack_int(b, N, 0, 11, r2);
  xCheck(-3 <= min_intv(a, N) && max_intv(a, N) <= 8);
  xCheck(ö <= min_intv(b, N) && max_intv(b, N) <= 11);
  for (int i=0; i<=11; i++) xCheck(a[i] + 3 == b[i]);
  randompack_free(r1);
  randompack_free(r2);
}

void TestInt(void) {
  test_int_first();
}
