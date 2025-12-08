// -*- C -*-
// Basic tests for randompack_uint32: bounds, determinism, seed variation.
#include <stdbool.h>
#include <stdint.h>

#include "randompack.h"
#include "printX.h"
#include "TestUtil.h"
#include "xCheck.h"

// Helper: create an RNG and fill n uint32 values with a given bound.
static void draw_randoms(char *engine, uint32_t *x, int n, uint32_t bound, int seed) {
  randompack_rng *rng = create_seeded_rng(engine, seed);
  xCheck(rng);
  bool ok = randompack_uint32(x, n, bound, rng);
  xCheck(ok);
  randompack_free(rng);
}

// Check argument handling, including null / zero-length arguments
// Boundary-condition tests for randompack_uint32
static void test_edge_cases(char *engine) {
  uint32_t buf[4]      = {0xDEAD, 0xBEEF, 0xCAFE, 0xFEED};
  uint32_t original[4] = {0xDEAD, 0xBEEF, 0xCAFE, 0xFEED};
  bool ok;
  randompack_rng *rng = create_seeded_rng(engine, 333);
  ok = randompack_uint32(buf, 0, 0, rng); check_success(ok, rng); // n = 0
  xCheck(equal_vec32(buf, original, 4));                          // –doesn't touch buffer
  ok = randompack_uint32(0, 4, 0, rng);   check_failure(ok, rng); // NULL buffer w/n > 0
  ok = randompack_uint32(buf, 4, 0, 0);   xCheck(!ok);            // NULL rng
  ok = randompack_uint32(buf, 4, 0, rng); check_success(ok, rng); // normal call
  ok = randompack_uint32(buf, 4, UINT32_MAX, rng); check_success(ok, rng); // max bound
  randompack_free(rng);
}

// Unbounded draws with same seed/engine must match.
static void test_unbounded_determinism(char *engine) {
  uint32_t a[3], b[3], c[3];
  draw_randoms(engine, a, LEN(a), 0, 42);
  draw_randoms(engine, b, LEN(b), 0, 42);
  draw_randoms(engine, c, LEN(c), 0, 43);
  xCheckMsg(equal_vec32(a, b, LEN(a)), engine);
  xCheckMsg(a[0] != c[0] || a[1] != b[1], engine); // "or" to maintain "7 sigma tests"
}

// Bounded large-sample sanity check: counts across buckets are balanced.
static void test_balanced_counts(char *engine) {
  uint32_t *x;
  int n = N_BAL_CNTS;
  xCheck(ALLOC(x, n));
  randompack_rng *rng = create_seeded_rng(engine, 42);
  xCheck(rng);
  uint32_t bounds[] = {5, 10};
  for (int b = 0; b < LEN(bounds); b++) {
    uint32_t bound = bounds[b];
    int counts[10] = {0};
    bool ok = randompack_uint32(x, n, bound, rng);
    xCheck(ok);
	 xCheck(maxv32(x, n) < bound);
    for (int i = 0; i < n; i++) counts[x[i]]++;
    xCheckMsg(check_balanced_counts(counts, bound), engine);
  }
  randompack_free(rng);
  FREE(x);
}

static inline bool bitset(uint32_t x, int b) {
  return x & (1u << b);
}

// Balanced bits check. Check that all bits appear approximately equally often
static void test_balanced_bits(char *engine) {
  uint32_t *x;
  int n = N_BAL_BITS;
  xCheck(ALLOC(x, n));
  randompack_rng *rng = create_seeded_rng(engine, 44);
  xCheck(rng);
  bool ok = randompack_uint32(x, n, 0, rng); // bound = 0 => unbounded
  xCheck(ok);
  int ones[32] = {0};
  for (int i = 0; i < n; i++) {
    for (int b = 0; b < 32; b++) {
      if (bitset(x[i], b)) ones[b]++;
    }
  }
  bool balanced_bits = check_balanced_bits(ones, n, 32);
  xCheckMsg(balanced_bits, engine);
  randompack_free(rng);
  FREE(x);
}

void TestUint32(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
	 printS("\nTesting Uint32 with engine", e);
    test_edge_cases(e);
    test_unbounded_determinism(e);
    test_balanced_counts(e);
    test_balanced_bits(e);
  }
}
