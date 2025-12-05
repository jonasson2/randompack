// -*- C -*-
// Basic tests for randompack_uint32: bounds, determinism, seed variation.
#include <stdbool.h>
#include <stdint.h>

#include "randompack.h"
#include "xCheck.h"

// Helper: create an RNG and fill n uint32 values with a given bound.
static void fill_uint32(const char *engine, int seed, uint32_t bound, uint32_t *x, int n)
                        {
  randompack_rng *rng = randompack_create(engine, seed);
  xCheck(rng);
  bool ok = randompack_uint32(x, n, bound, rng);
  xCheck(ok);
  randompack_free(rng);
}

// Bounded draws must be strictly below the bound.
static void test_bound_range(void) {
  const uint32_t bound = 10;
  uint32_t x[8];
  fill_uint32("x256++", 123, bound, x, 8);
  for (int i = 0; i < 8; i++) {
    xCheck(x[i] < bound);
  }
}

// Unbounded draws with same seed/engine must match.
static void test_unbounded_determinism(void) {
  uint32_t a[4];
  uint32_t b[4];
  fill_uint32("x256++", 7, 0, a, 4);
  fill_uint32("x256++", 7, 0, b, 4);
  for (int i = 0; i < 4; i++) {
    xCheck(a[i] == b[i]);
  }
}

// Different seeds should produce different unbounded sequences (very likely).
static void test_seed_changes_output(void) {
  uint32_t a[4];
  uint32_t b[4];
  fill_uint32("x256++", 1, 0, a, 4);
  fill_uint32("x256++", 2, 0, b, 4);
  bool diff = false;
  for (int i = 0; i < 4; i++) {
    if (a[i] != b[i]) diff = true;
  }
  xCheck(diff);
}

// Unbounded draws should produce something non-trivial.
static void test_unbounded_nonzero(void) {
  uint32_t x[2];
  fill_uint32("x256++", 99, 0, x, 2);
  xCheck(x[0] || x[1]); // should not both be zero
}

void TestUint32(void) {
  test_bound_range();
  test_unbounded_determinism();
  test_seed_changes_output();
  test_unbounded_nonzero();
}
