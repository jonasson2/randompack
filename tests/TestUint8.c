// -*- C -*-
// Basic tests for randompack_uint32: bounds, determinism, seed variation.
#include <stdbool.h>
#include <stdint.h>

#include "randompack.h"
#include "printX.h"
#include "test_util.h"
#include "xCheck.h"

// Helper: create an RNG and fill n uint8 values with a given bound.
static void draw_randoms(char *engine, uint8_t *x, int n, uint8_t bound, int seed) {
  randompack_rng *rng = create_seeded_rng(engine, seed);
  ASSERT(rng);
  bool ok = randompack_uint8(x, n, bound, rng);
  ASSERT(ok);
  randompack_free(rng);
}

static void two_part_draw(char *engine, uint8_t *x, int n1, int n2, int seed) {
  randompack_rng *rng = create_seeded_rng(engine, seed);
  ASSERT(rng);
  ASSERT(randompack_uint8(x, n1, 0, rng));
  ASSERT(randompack_uint8(x + n1, n2, 0, rng));
  randompack_free(rng);
}

static void three_part_draw(char *engine, uint8_t *x, int n1, int n2, int n3, int seed) {
  randompack_rng *rng = create_seeded_rng(engine, seed);
  ASSERT(rng);
  ASSERT(randompack_uint8(x, n1, 0, rng));
  ASSERT(randompack_uint8(x + n1, n2, 0, rng));
  ASSERT(randompack_uint8(x + n1 + n2, n3, 0, rng));
  randompack_free(rng);
}

static void test_edge_cases(char *engine) {
  // Check argument handling, including null / zero-length arguments
  // Boundary-condition tests for randompack_uint8
  uint8_t buf[4]      = {1, 2, 3, 4};
  uint8_t original[4] = {1, 2, 3, 4};
  bool ok;
  randompack_rng *rng = create_seeded_rng(engine, 333);
  ok = randompack_uint8(buf, 0, 0, rng); check_success(ok, rng); // n = 0
  xCheck(equal_vec8(buf, original, 4));                          // –doesn't touch buffer
  ok = randompack_uint8(0, 4, 0, rng);   check_failure(ok, rng); // NULL buffer w/n > 0
  ok = randompack_uint8(buf, 4, 0, 0);   xCheck(!ok);            // NULL rng
  ok = randompack_uint8(buf, 4, 0, rng); check_success(ok, rng); // normal call
  ok = randompack_uint8(buf, 4, UINT8_MAX, rng); check_success(ok, rng); // max bound
  randompack_free(rng);
}

static void test_unbounded_determinism(char *engine) {
  // Unbounded draws with same seed/engine must match, also when drawn in two parts.
  uint8_t a[10], b[10], c[10], d[10], e[10];
  draw_randoms(engine, a, LEN(a), 0, 42);
  draw_randoms(engine, b, LEN(b), 0, 42);
  draw_randoms(engine, c, LEN(c), 0, 43);
  two_part_draw(engine, d, 3, 8, 42);
  three_part_draw(engine, e, 2, 5, 3, 42);
  xCheckMsg(equal_vec8(a, b, LEN(a)), engine);
  xCheckMsg(equal_vec8(a, d, LEN(a)), engine);  
  xCheckMsg(equal_vec8(a, e, LEN(a)), engine);  
  xCheckMsg(!equal_vec8(a, c, 5), engine);
}

// Bounded large-sample sanity check: counts across buckets are balanced.
static void test_balanced_counts(char *engine) {
  uint8_t *x;
  int n = N_BAL_CNTS;
  TEST_ALLOC(x, n);
  randompack_rng *rng = create_seeded_rng(engine, 42);
  ASSERT(rng);
  uint8_t bounds[] = {5, 10};
  for (int b = 0; b < LEN(bounds); b++) {
    uint8_t bound = bounds[b];
    int counts[10] = {0};
    bool ok = randompack_uint8(x, n, bound, rng);
    ASSERT(ok);
	 xCheck(maxv8(x, n) < bound);
    for (int i = 0; i < n; i++) counts[x[i]]++;
    xCheckMsg(check_balanced_counts(counts, bound), engine);
  }
  randompack_free(rng);
  FREE(x);
}

static inline bool bitset(uint8_t x, int b) {
  return x & (1u << b);
}

// Balanced bits check. Check that all bits appear approximately equally often
static void test_balanced_bits(char *engine) {
  uint8_t *x;
  int n = N_BAL_BITS;
  TEST_ALLOC(x, n);
  randompack_rng *rng = create_seeded_rng(engine, 44);
  ASSERT(rng);
  bool ok = randompack_uint8(x, n, 0, rng); // bound = 0 => unbounded
  ASSERT(ok);
  int ones[8] = {0};
  for (int i = 0; i < n; i++) {
    for (int b = 0; b < 8; b++) {
      if (bitset(x[i], b)) ones[b]++;
    }
  }
  bool balanced_bits = check_balanced_bits(ones, n, 8);
  xCheckMsg(balanced_bits, engine);
  randompack_free(rng);
  FREE(x);
}

void TestUint8(void) {
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    char *e = engines[i];
	 printS("\nTesting Uint8 with engine", e);
    test_edge_cases(e);
    test_unbounded_determinism(e);
    test_balanced_counts(e);
    test_balanced_bits(e);
  }
  free_engines(engines, n);
}
