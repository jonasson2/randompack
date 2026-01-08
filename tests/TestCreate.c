// -*- C -*-
// Tests for create_seeded_rng and engine-name handling (defaults, system CSPRNG, and
// error reporting).
#include <stdint.h>
#include <stdbool.h>
#include "randompack_config.h"
#include "randompack.h"
#include "printX.h"
#include "xCheck.h"
#include "TestUtil.h"

// Return the first n uint64 draws from the given engine with fixed seed 123.
// Check that everything works cleanly (unbounded draw)
static void draw_randoms(char *engine, uint64_t *x, int n, uint64_t seed) {
  randompack_rng *rng = create_seeded_rng(engine, seed);
  check_rng_clean(rng);
  bool ok = randompack_uint64(x, n, 0, rng);
  check_success(ok, rng);
  print64(engine, x[0]);  // keep logging the first element
  randompack_free(rng);
}

// Check that identical engines agree and different engines differ.
static void test_determinism(void) {
  enum { LEN_STREAM = 4, NENGINES = LEN(engines) };
  uint64_t x[NENGINES][LEN_STREAM], y[NENGINES][LEN_STREAM],
           z[NENGINES][LEN_STREAM];
  for (int i=0; i<NENGINES; i++) {
    draw_randoms(engines[i], x[i], LEN_STREAM, 42);
    draw_randoms(engines[i], y[i], LEN_STREAM, 42);
    draw_randoms(engines[i], z[i], LEN_STREAM, 43);
    xCheck(equal_vec64(x[i], y[i], LEN_STREAM));
    xCheck(everywhere_different(x[i], z[i], LEN_STREAM));
    for (int j = i+1; j < NENGINES; j++) { // all the later engines
      draw_randoms(engines[j], y[j], LEN_STREAM, 42);
      xCheck(everywhere_different(x[i], y[j], LEN_STREAM));
    }
  }
}

// Unknown engine names should yield a non-null "invalid" rng object with a non-blank
// last_error. Drawing from an invalid rng must fail and set another non-blank error.
static void test_bad_engine_name(void) {
  randompack_rng *rng = randompack_create("garbage"); // garbage name
  ASSERT(rng);
  char *err = randompack_last_error(rng);
  xCheck(err && err[0]); // non-null, non-blank
  bool ok = randompack_uint64(0, 1, 0, rng); // null output buffer
  check_failure(ok, rng);
  printS("create_seeded_rng with engine", "garbage");
  printS("last error", err);
  printMsg("randompack_uint64 with null output buffer");
  printS("last error", randompack_last_error(rng));
  randompack_free(rng);
}

// Null engine name --> default engine which produces valid output without error.
static void test_null_engine_name(void) {
  randompack_rng *rng = create_seeded_rng(0, 123);
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
  draw_randoms(0, a, 1, 42);
  draw_randoms("x256++", b, 1, 42);
  xCheck(a[0] == b[0]);
}

#if !HAVE128 || !HAVE128MUL
static void test_engine_unavailable(char *engine) {
  // Check that the specified engine results in an invalid RNG
  randompack_rng *rng = randompack_create(engine);
  ASSERT(rng);
  char *err = randompack_last_error(rng);
  xCheck(err && err[0]);
  randompack_free(rng);
}
#endif

// "system" should map to the system CSPRNG, produce non-trivial output, and set no error.
static void test_system_engine(void) {
  randompack_rng *rng = create_seeded_rng("system", 0);
  check_rng_clean(rng);
  uint64_t x[2] = {0, 0};
  bool ok = randompack_uint64(x, 2, 0, rng);
  check_success(ok, rng);
  xCheck(x[0] || x[1]);     // must produce something nreon-trivial
  printS("system engine", "system");
  randompack_free(rng);
}

void TestCreate(void) {
  test_determinism();
  test_bad_engine_name();
  test_null_engine_name();
  test_default_engine_matches_x256pp();
  test_system_engine();
#if !HAVE128
  test_engine_unavailable("pcg64");
  test_engine_unavailable("cwg128");
#endif
#if !HAVE128MUL
  test_engine_unavailable("philox");
#endif
}
