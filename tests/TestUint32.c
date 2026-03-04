// -*- C -*-
// Basic tests for randompack_uint32: bounds, determinism, seed variation.
#include <stdbool.h>
#include <stdint.h>

#include "randompack.h"
#include "printX.h"
#include "test_util.h"
#include "xCheck.h"

// Helper: create an RNG and fill n uint32 values with a given bound.
static void draw_randoms(char *engine, uint32_t *x, int n, uint32_t bound, int seed) {
  randompack_rng *rng = create_seeded_rng(engine, seed);
  ASSERT(rng);
  bool ok = randompack_uint32(x, n, bound, rng);
  ASSERT(ok);
  randompack_free(rng);
}

static void two_part_draw(char *engine, uint32_t *x, int n1, int n2, int seed) {
  randompack_rng *rng = create_seeded_rng(engine, seed);
  ASSERT(rng);
  ASSERT(randompack_uint32(x, n1, 0, rng));
  ASSERT(randompack_uint32(x + n1, n2, 0, rng));
  randompack_free(rng);
}

static void test_edge_cases(char *engine) {
  // Check argument handling, including null / zero-length arguments
  // Boundary-condition tests for randompack_uint32
  uint32_t buf[4]      = {0xDEAD, 0xBEEF, 0xCAFE, 0xFEED};
  uint32_t original[4] = {0xDEAD, 0xBEEF, 0xCAFE, 0xFEED};
  bool ok;
  randompack_rng *rng = create_seeded_rng(engine, 333);
  ok = randompack_uint32(buf, 0, 0, rng); check_success(ok, rng); // n = 0
  CHECK_EQUALV(buf, original, 4);                    // –doesn't touch buffer
  ok = randompack_uint32(0, 4, 0, rng);   check_failure(ok, rng); // NULL buffer w/n > 0
  ok = randompack_uint32(buf, 4, 0, 0);   xCheck(!ok);            // NULL rng
  ok = randompack_uint32(buf, 4, 0, rng); check_success(ok, rng); // normal call
  ok = randompack_uint32(buf, 4, UINT32_MAX, rng); check_success(ok, rng); // max bound
  randompack_free(rng);
}

static void test_mixed_draw(char *engine) {
  uint8_t byte[5];
  uint32_t a[2];
  uint32_t b[5];
  randompack_rng *rng = create_seeded_rng(engine, 42);
  ASSERT(rng);
  ASSERT(randompack_uint8(byte, 5, 0, rng)); // leaves just one uint16 in buf64
  ASSERT(randompack_uint32(a, 2, 0, rng));
  randompack_free(rng);
  rng = create_seeded_rng(engine, 42);
  ASSERT(rng);
  ASSERT(randompack_uint32(b, 4, 0, rng));
  CHECK_EQUALV(a, b + 2, 2);
  randompack_free(rng);
}

static void test_unbounded_determinism(char *engine) {
  // Unbounded draws with same seed/engine must match, also when drawn in two parts.
  uint32_t a[5], b[5], c[5], d[5];
  draw_randoms(engine, a, LEN(a), 0, 42);
  draw_randoms(engine, b, LEN(b), 0, 42);
  draw_randoms(engine, c, LEN(c), 0, 43);
  two_part_draw(engine, d, 3, 2, 42);
  CHECK_EQUALV_MSG(a, b, LEN(a), engine);
  CHECK_EQUALV_MSG(a, d, LEN(a), engine);
  CHECK_DIFFV_MSG(a, c, 2, engine);
}

// Bounded large-sample sanity check: counts across buckets are balanced.
static void test_balanced_counts(char *engine) {
  uint32_t *x;
  int n = N_BAL_CNTS;
  TEST_ALLOC(x, n);
  randompack_rng *rng = create_seeded_rng(engine, 42);
  ASSERT(rng);
  uint32_t bounds[] = {5, 10};
  for (int b = 0; b < LEN(bounds); b++) {
    uint32_t bound = bounds[b];
    int counts[10] = {0};
    bool ok = randompack_uint32(x, n, bound, rng);
    ASSERT(ok);
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
  TEST_ALLOC(x, n);
  randompack_rng *rng = create_seeded_rng(engine, 44);
  ASSERT(rng);
  bool ok = randompack_uint32(x, n, 0, rng); // bound = 0 => unbounded
  ASSERT(ok);
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
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    char *e = engines[i];
	 printS("\nTesting Uint32 with engine", e);
    test_edge_cases(e);
    test_unbounded_determinism(e);
    test_balanced_counts(e);
    test_balanced_bits(e);
	 test_mixed_draw(e);
  }
  free_engines(engines, n);
}

void TestUint32x(char *engine) {
  char *e = engine;
  printS("\nTesting Uint32 with engine", e);
  test_edge_cases(e);
  test_unbounded_determinism(e);
  test_balanced_counts(e);
  test_balanced_bits(e);
  test_mixed_draw(e);
}
