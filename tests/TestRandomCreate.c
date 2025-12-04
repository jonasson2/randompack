// -*- C -*-
// Tests for randompack_create and engine-name handling (aliases, defaults, system CSPRNG,
// Park-Miller behavior, and error reporting).
#include <stdint.h>
#include <stdbool.h>
#include "randompack_config.h"
#include "randompack.h"
#include "printX.h"
#include "xCheck.h"
#include "TestUtil.h"

typedef struct {
  const char *full;
  const char *abbrev;
} engine_entry;
// Test helper: verify that an RNG object is non-null and last_error is empty.
static void check_rng_clean(randompack_rng *rng) {
  xCheck(rng);
  const char *err = randompack_last_error(rng);
  xCheck(!err || !err[0]);
}

// Test helper: verify that an RNG call succeeded and last_error is empty.
static void check_success(bool ok, randompack_rng *rng) {
  xCheck(ok);
  const char *err = randompack_last_error(rng);
  xCheck(!err || !err[0]);
}

// Test helper: verify that an RNG call failed and produced a non-empty error.
static void check_failure(bool ok, randompack_rng *rng) {
  xCheck(!ok);
  const char *err = randompack_last_error(rng);
  xCheck(err && err[0]);
}

// Return the first n uint64 draws from the given engine with fixed seed 123.
// Check that everything works cleanly.
static void first_uint64_seq(const char *engine, uint64_t *x, int n) {
  randompack_rng *rng = randompack_create(engine, 123);
  check_rng_clean(rng);
  bool ok = randompack_uint64(x, n, 0, rng);
  check_success(ok, rng);
  print64(engine, x[0]);  // keep logging the first element
  randompack_free(rng);
}

// Check that full engine names and their abbreviations produce identical first outputs
// (same seed), and that different engines differ.
static void test_engine_aliases(void) {
  engine_entry engines[] = {
    { "xorshift128+", "x128+" },
    { "xoshiro256**", "x256**" },
    { "xoshiro256++", "x256++" },
    { "chacha20",     "chacha20" },
#ifdef HAVE128
    { "pcg64",        "pcg"     },
#endif
  };
  const int n = (int)(sizeof engines/sizeof engines[0]);
  const int m = 4;
  uint64_t full[n][m];
  uint64_t abbr[n][m];
  // alias equality
  for (int i=0; i<n; i++) {
    first_uint64_seq(engines[i].full, full[i], m);
    first_uint64_seq(engines[i].abbrev, abbr[i], m);
    CHECK_VEC_EQ(full[i], abbr[i]);
  }
  // different engines must differ
  for (int i=0; i<n; i++) {
    for (int j=i+1; j<n; j++) {
      CHECK_VEC_NEQ(full[i], full[j]);
    }
  }
}

// Unknown engine names should yield a non-null "invalid" rng object with a non-blank
// last_error. Drawing from an invalid rng must fail and set another non-blank error.
static void test_bad_engine_name(void) {
  randompack_rng *rng = randompack_create("garbage", 123); // garbage name
  xCheck(rng);
  const char *err = randompack_last_error(rng);
  xCheck(err && err[0]); // non-null, non-blank
  bool ok = randompack_uint64(0, 1, 0, rng); // null output buffer
  check_failure(ok, rng);
  const char *err2 = randompack_last_error(rng);
  printS("randompack_create with engine", "garbage");
  printS("last error", err);
  printMsg("randompack_uint64 with null output buffer");
  printS("last error", err2);
  randompack_free(rng);
}

// Null engine name --> default engine which produces valid output without error.
static void test_null_engine_name(void) {
  randompack_rng *rng = randompack_create(0, 123);
  check_rng_clean(rng);
  uint64_t x = 0;
  bool ok = randompack_uint64(&x, 1, 0, rng);
  check_success(ok, rng);
  randompack_free(rng);
}

// Null engine name should behave like explicitly requesting the default
// engine ("x256++" at present), for the same seed.
static void test_default_engine_matches_x256pp(void) {
  uint64_t a[1], b[1];
  first_uint64_seq(0, a, 1);
  first_uint64_seq("x256++", b, 1);
  xCheck(a[0] == b[0]);
}

#ifndef HAVE128
static void test_pcg64_unavailable(void) {
  const char *names[] = { "pcg64", "pcg" };
  for (int i = 0; i < (int)(sizeof names/sizeof names[0]); i++) {
    randompack_rng *rng = randompack_create(names[i], 123);
    xCheck(rng);
    const char *err = randompack_last_error(rng);
    xCheck(err && err[0]);
    uint64_t x = 0;
    bool ok = randompack_uint64(&x, 1, 0, rng);
    check_failure(ok, rng);
    printS("pcg engine (unavailable)", names[i]);
    printS("last error", err);
    randompack_free(rng);
  }
}
#endif

// "system" and "system-csprng" should both map to the system CSPRNG, produce non-trivial
// output, and set no error.
static void test_system_engine(void) {
  const char *names[] = { "system", "system-csprng" };
  for (int i = 0; i < (int)(sizeof names/sizeof names[0]); i++) {
    randompack_rng *rng = randompack_create(names[i], 0);
    check_rng_clean(rng);
    uint64_t x[2] = {0, 0};
    bool ok = randompack_uint64(x, 2, 0, rng);
    check_success(ok, rng);
    xCheck(x[0] || x[1]);     // must produce something nreon-trivial
    printS("system engine", names[i]);
    randompack_free(rng);
  }
}

// Park-Miller supports only int draws; uint64 requests must fail.
static void test_park_miller(void) {
  randompack_rng *rng = randompack_create("park-miller", 123);
  check_rng_clean(rng);
  uint64_t x = 0;
  bool ok = randompack_uint64(&x, 1, 0, rng);
  check_failure(ok, rng);
  randompack_free(rng);
}

// Park-Miller determinism for seeds: same seed => same int, different seed => different
// int (using full and abbrev engine names).
static void test_park_miller_determinism(void) {
  int a = 0;
  int b = 0;
  int c = 0;
  randompack_rng *r1 = randompack_create("park-miller", 42);
  randompack_rng *r2 = randompack_create("pm", 42);
  randompack_rng *r3 = randompack_create("pm", 43);
  check_rng_clean(r1);
  check_rng_clean(r2);
  check_rng_clean(r3);
  bool ok1 = randompack_int(&a, 1, 0, 100, r1);
  bool ok2 = randompack_int(&b, 1, 0, 100, r2);
  bool ok3 = randompack_int(&c, 1, 0, 100, r3);
  check_success(ok1, r1);
  check_success(ok2, r2);
  check_success(ok3, r3);
  xCheck(a == b);
  xCheck(a != c);
  randompack_free(r1);
  randompack_free(r2);
  randompack_free(r3);
}

void TestRandomCreate(void) {
  RUN_TEST(engine_aliases);
  RUN_TEST(bad_engine_name);
  RUN_TEST(null_engine_name);
  RUN_TEST(default_engine_matches_x256pp);
#ifndef HAVE128
  RUN_TEST(pcg64_unavailable);
#endif
  RUN_TEST(system_engine);
  RUN_TEST(park_miller);
  RUN_TEST(park_miller_determinism);
}
