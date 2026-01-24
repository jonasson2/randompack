// -*- C -*-
// Basic tests for randompack_uint64: bounds, determinism, seed variation.
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "TestUtil.h"
#include "xCheck.h"

static void draw_bounded_randoms(char *engine, uint64_t *x, int len, uint64_t bound,
                                 int seed) {
  randompack_rng *rng = create_seeded_rng(engine, seed);
  ASSERT(rng);
  bool ok = randompack_uint64(x, len, bound, rng);
  ASSERT(ok);
  randompack_free(rng);
}

static void draw_unbounded_randoms(char *engine, uint64_t *x, int len, int seed) {
  bool ok;
  randompack_rng *rng = create_seeded_rng(engine, seed);
  ASSERT(rng);
  ok = randompack_uint64(x, len, 0, rng);
  ASSERT(ok);
  randompack_free(rng);
}

static void test_edge_cases(char *engine) {
  uint64_t buf[4] = {0xDEADBEEF, 0xCAFEBABE, 0x0BADF00D, 0xFEEDFACE};
  uint64_t original[4] = {0xDEADBEEF, 0xCAFEBABE, 0x0BADF00D, 0xFEEDFACE};
  bool ok;
  randompack_rng *rng = create_seeded_rng(engine, 333);
  ok = randompack_uint64(buf, 0, 0, rng); check_success(ok, rng); // len = 0
  xCheck(equal_vec64(buf, original, 4));                          // –doesn't touch buffer
  ok = randompack_uint64(0, 4, 0, rng);   check_failure(ok, rng); // NULL buffer w/len > 0
  ok = randompack_uint64(buf, 4, 0, 0);   xCheck(!ok);            // NULL rng
  ok = randompack_uint64(buf, 4, 0, rng); check_success(ok, rng); // normal call
  ok = randompack_uint64(buf, 4, UINT64_MAX, rng); check_success(ok, rng); // max bound
  randompack_free(rng);
}

// See test_determinism in TestCreate

static void test_unbounded_nonzero(char *engine) {
  uint64_t x[1];
  draw_unbounded_randoms(engine, x, LEN(x), 123);
  xCheckMsg(x[0] != 0, engine);
}

static void test_balanced_counts(char *engine) {
  uint64_t *x;
  int n = N_BAL_CNTS;
  TEST_ALLOC(x, n);  
  uint64_t bounds[] = {5, 10};
  for (int b = 0; b < LEN(bounds); b++) {
    uint64_t bound = bounds[b];
    int counts[10] = {0};
    draw_bounded_randoms(engine, x, n, bound, 42);
	 xCheck(maxv64(x, n) < bound);
    for (int i = 0; i < n; i++) counts[x[i]]++;
    xCheckMsg(check_balanced_counts(counts, bound), engine);
  }
  FREE(x);
}

static inline bool bitset64(uint64_t x, int b) {
  return x & (1ull << b);
}

static void test_balanced_bits(char *engine) {
  uint64_t *x;
  int n = N_BAL_BITS;
  TEST_ALLOC(x, n);
  draw_unbounded_randoms(engine, x, n, 45);
  int ones[64] = {0};
  for (int i = 0; i < n; i++) {
    for (int b = 0; b < 64; b++) {
      if (bitset64(x[i], b)) ones[b]++;
    }
  }
  xCheckMsg(check_balanced_bits(ones, n, 64), engine);
  FREE(x);
}

void TestUint64(void) {
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    char *e = engines[i];
	 printS("\nTesting Uint64 with engine", e);
    test_edge_cases(e);
    test_unbounded_nonzero(e);
    test_balanced_counts(e);
    test_balanced_bits(e);
  }
  free_engines(engines, n);
}
