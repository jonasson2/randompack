// -*- C -*-
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "randompack.h"
#include "printX.h"
#include "randompack_config.h"
#include "TestUtil.h"
#include "xCheck.h"

// Helper: create an RNG and fill n ints in [min, max].
static void draw_randoms(char *engine, int *x, int n, int min, int max, int seed) {
  randompack_rng *rng = create_seeded_rng(engine, seed);
  ASSERT(rng);
  bool ok = randompack_int(x, n, min, max, rng);
  check_success(ok, rng);
  randompack_free(rng);
}

// Basic bounded range checks comparing shifted streams.
static void test_int_simple(void) {
  enum { N = 128 };
  int a[N], b[N];
  draw_randoms("x256++", a, N, -3, 8, 99);
  draw_randoms("x256++", b, N, 0, 11, 99);
  xCheck(-3 <= minv(a, N) && maxv(a, N) <= 8);
  xCheck(0 <= minv(b, N) && maxv(b, N) <= 11);
  for (int i = 0; i < 12; i++) xCheck(a[i] + 3 == b[i]);
}

// Edge cases: zero-length, null buffer/rng, and bad bounds, max span.
static void test_edge_cases(char *engine, int max) {
  int buf[4] = {1, 2, 3, 4};
  int orig[4] = {1, 2, 3, 4};
  bool ok;
  randompack_rng *rng = create_seeded_rng(engine, 123);
  ok = randompack_int(buf, 0, 0, 10, rng); check_success(ok, rng); // len = 0
  xCheck(equal_vec(buf, orig, 4));
  ok = randompack_int(0, 4, 0, 10, rng);    check_failure(ok, rng); // null buffer w/len>0
  ok = randompack_int(buf, 4, 0, max, rng); check_success(ok, rng); // max span   
  ok = randompack_int(buf, 4, -1, max, rng);check_success(ok, rng); // max span + 1
  ok = randompack_int(buf, 4, 0, 10, 0);    xCheck(!ok);            // null rng
  ok = randompack_int(buf, 4, 5, 3, rng);   check_failure(ok, rng); // empty desired range
  randompack_free(rng);
}

// Different seeds should produce different bounded sequences.
static void test_seed_changes_output(char *engine) {
  int a[6];
  int b[6];
  draw_randoms(engine, a, LEN(a), -5, 5, 42);
  draw_randoms(engine, b, LEN(b), -5, 5, 43);
  xCheck(!equal_vec(a, b, LEN(a)));
}

// Balanced bits check for large positive int range.
// We test that bits 0..30 appear approx 50/50 when drawing from [0, INT_MAX - 2].
static void test_balanced_bits(char *engine) {
  const int N = N_BAL_BITS;
  int *x;
  TEST_ALLOC(x, N);
  randompack_rng *rng = create_seeded_rng(engine, 44);
  check_rng_clean(rng);
  bool ok = randompack_int(x, N, 0, INT_MAX - 2, rng);
  check_success(ok, rng);
  int ones[31] = {0};
  for (int i=0; i<N; i++) {
    uint32_t u = x[i];
    for (int b=0; b<31; b++) {
      if (u & (1u << b)) ones[b]++;
    }
  }
  xCheckMsg(check_balanced_bits(ones, N, 31), engine);
  randompack_free(rng);
  FREE(x);
}

void TestInt(void) {
  test_int_simple();
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    char *e = engines[i];
    test_edge_cases(e, INT_MAX);
    test_seed_changes_output(e);
    test_balanced_bits(e);
  }
  free_engines(engines, n);
}
