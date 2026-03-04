// -*- C -*-
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>

#include "randompack.h"
#include "printX.h"
#include "randompack_config.h"
#include "test_util.h"
#include "xCheck.h"

// Helper: create an RNG and fill n long long in [min, max].
static void draw_randoms(char *engine, long long *x, int n,
  long long min, long long max, int seed) {
  randompack_rng *rng = create_seeded_rng(engine, seed);
  ASSERT(rng);
  bool ok = randompack_long_long(x, n, min, max, rng);
  check_success(ok, rng);
  randompack_free(rng);
}

static void test_long_long_simple(void) {
  enum { N = 128 };
  long long a[N], b[N];
  draw_randoms("x256++", a, N, -3, 8, 99);
  draw_randoms("x256++", b, N, 0, 11, 99);
  TEST_SUPPORT(long long, a, N, -3, 8);
  TEST_SUPPORT(long long, b, N, 0, 11);
  bool same = true;
  for (int i = 0; i < N; i++) {
    if (a[i] + 3 != b[i]) {
      same = false;
      break;
    }
  }
  xCheck(same);
}

static void test_edge_cases(char *engine) {
  long long buf[4] = {1, 2, 3, 4};
  long long orig[4] = {1, 2, 3, 4};
  bool ok;
  randompack_rng *rng = create_seeded_rng(engine, 123);
  ok = randompack_long_long(buf, 0, 0, 10, rng); check_success(ok, rng);
  CHECK_EQUALV(buf, orig, LEN(buf));
  ok = randompack_long_long(0, 4, 0, 10, rng);  check_failure(ok, rng);
  ok = randompack_long_long(buf, 4, LLONG_MIN, LLONG_MAX, rng);
  check_success(ok, rng);
  ok = randompack_long_long(buf, 4, 5, 3, rng); check_failure(ok, rng);
  ok = randompack_long_long(buf, 4, 0, 10, 0);  xCheck(!ok);
  randompack_free(rng);
}

static void test_seed_changes_output(char *engine) {
  long long a[6];
  long long b[6];
  draw_randoms(engine, a, LEN(a), -5, 5, 42);
  draw_randoms(engine, b, LEN(b), -5, 5, 43);
  bool same = true;
  for (int i = 0; i < LEN(a); i++) {
    if (a[i] != b[i]) {
      same = false;
      break;
    }
  }
  xCheck(!same);
}

static void test_large_ranges(char *engine) {
  long long x[64];
  randompack_rng *rng = create_seeded_rng(engine, 321);
  bool ok = randompack_long_long(x, LEN(x), LLONG_MAX - 1000, LLONG_MAX - 1, rng);
  check_success(ok, rng);
  TEST_SUPPORT(long long, x, LEN(x), LLONG_MAX - 1000, LLONG_MAX - 1);
  ok = randompack_long_long(x, LEN(x), LLONG_MIN + 1, LLONG_MIN + 1000, rng);
  check_success(ok, rng);
  TEST_SUPPORT(long long, x, LEN(x), LLONG_MIN + 1, LLONG_MIN + 1000);
  randompack_free(rng);
}

void TestLongLong(void) {
  test_long_long_simple();
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    char *e = engines[i];
    test_edge_cases(e);
    test_seed_changes_output(e);
    test_large_ranges(e);
  }
  free_engines(engines, n);
}
