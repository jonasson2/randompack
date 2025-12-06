// -*- C -*-
// Basic tests for randompack_uint64: bounds, determinism, seed variation.
#include <stdbool.h>
#include <stdint.h>

#include "randompack.h"
#include "printX.h"
#include "TestUtil.h"
#include "xCheck.h"

static void draw_randoms(char *engine, uint64_t *x, int n, uint64_t bound, int seed) {
  randompack_rng *rng = randompack_create(engine, seed);
  xCheck(rng);
  bool ok = randompack_uint64(x, n, bound, rng);
  xCheck(ok);
  randompack_free(rng);
}

static void test_edge_cases(char *engine) {
  uint64_t buf[4] = {0xDEADBEEF, 0xCAFEBABE, 0x0BADF00D, 0xFEEDFACE};
  uint64_t original[4] = {0xDEADBEEF, 0xCAFEBABE, 0x0BADF00D, 0xFEEDFACE};
  bool ok;
  randompack_rng *rng = randompack_create(engine, 333);
  ok = randompack_uint64(buf, 0, 0, rng); check_success(ok, rng); // n = 0
  xCheck(equal_vec64(buf, original, 4));                         // –doesn't touch buffer
  ok = randompack_uint64(0, 4, 0, rng);   check_failure(ok, rng); // NULL buffer w/n > 0
  ok = randompack_uint64(buf, 4, 0, 0);   xCheck(!ok);            // NULL rng
  ok = randompack_uint64(buf, 4, 0, rng); check_success(ok, rng); // normal call
  ok = randompack_uint64(buf, 4, UINT64_MAX, rng); check_success(ok, rng); // max bound
  randompack_free(rng);
}

static void test_unbounded_determinism(char *engine) {
  uint64_t a[4];
  uint64_t b[4];
  draw_randoms(engine, a, LEN(a), 0, 42);
  draw_randoms(engine, b, LEN(b), 0, 42);
  xCheckMsg(equal_vec64(a, b, LEN(a)), engine);
}

static void test_seed_changes_output(char *engine) {
  uint64_t a[4];
  uint64_t b[4];
  draw_randoms(engine, a, LEN(a), 0, 42);
  draw_randoms(engine, b, LEN(b), 0, 43);
  xCheckMsg(!equal_vec64(a, b, LEN(a)), engine);
}

static void test_unbounded_nonzero(char *engine) {
  uint64_t x[1];
  draw_randoms(engine, x, LEN(x), 0, 123);
  xCheckMsg(x[0], engine);
}

static void test_balanced_counts(char *engine) {
  uint64_t *x;
  int n = N_BAL_CNTS;
  xCheck(ALLOC(x, n));
  randompack_rng *rng = randompack_create(engine, 42);
  xCheck(rng);
  uint64_t bounds[] = {5, 10};
  for (int b = 0; b < LEN(bounds); b++) {
    uint64_t bound = bounds[b];
    int counts[10] = {0};
    bool ok = randompack_uint64(x, n, bound, rng);
    xCheck(ok);
	 xCheck(maxv64(x, n) < bound);
    for (int i = 0; i < n; i++) counts[x[i]]++;
    xCheckMsg(check_balanced_counts(counts, (int)bound), engine);
  }
  randompack_free(rng);
  FREE(x);
}

static inline bool bitset64(uint64_t x, int b) {
  return x & (1ull << b);
}

static void test_balanced_bits(char *engine) {
  uint64_t *x;
  int n = N_BAL_BITS;
  xCheck(ALLOC(x, n));
  randompack_rng *rng = randompack_create(engine, 44);
  xCheck(rng);
  bool ok = randompack_uint64(x, n, 0, rng);
  xCheck(ok);
  int ones[64] = {0};
  for (int i = 0; i < n; i++) {
    for (int b = 0; b < 64; b++) {
      if (bitset64(x[i], b)) ones[b]++;
    }
  }
  xCheckMsg(check_balanced_bits(ones, n, 64), engine);
  randompack_free(rng);
  FREE(x);
}

void TestUint64(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
	 printS("\nTesting Uint64 with engine", e);
    test_edge_cases(e);
    test_unbounded_determinism(e);
    test_seed_changes_output(e);
    test_unbounded_nonzero(e);	 
    test_balanced_counts(e);
    test_balanced_bits(e);
  }
}
