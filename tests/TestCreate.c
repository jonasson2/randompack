// -*- C -*-
// Tests for create_seeded_rng and engine-name handling (aliases, defaults, system CSPRNG,
// Park-Miller behavior, and error reporting).
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

// Check that abbreviated engine names work
static void test_engine_aliases(void) {
  enum { LEN_STREAM = 4, NENGINES = LEN(engines) };
  uint64_t x[NENGINES][LEN_STREAM], y[NENGINES][LEN_STREAM];
  for (int i=0; i<NENGINES; i++) {
    draw_randoms(engines[i], x[i], LEN_STREAM, 42);
    draw_randoms(abbrev[i], y[i], LEN_STREAM, 42);
    xCheck(equal_vec64(x[i], y[i], LEN_STREAM));
  }
}
  
// Unknown engine names should yield a non-null "invalid" rng object with a non-blank
// last_error. Drawing from an invalid rng must fail and set another non-blank error.
static void test_bad_engine_name(void) {
  randompack_rng *rng = randompack_create("garbage"); // garbage name
  xCheck(rng);
  char *err = randompack_last_error(rng);
  xCheck(err && err[0]); // non-null, non-blank
  bool ok = randompack_uint64(0, 1, 0, rng); // null output buffer
  check_failure(ok, rng);
  char *err2 = randompack_last_error(rng);
  printS("create_seeded_rng with engine", "garbage");
  printS("last error", err);
  printMsg("randompack_uint64 with null output buffer");
  printS("last error", err2);
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

#ifndef HAVE128
static void test_pcg64_unavailable(void) {
  char *names[] = { "pcg64", "pcg" };
  for (int i = 0; i < (int)(sizeof names/sizeof names[0]); i++) {
    randompack_rng *rng = create_seeded_rng(names[i], 123);
    xCheck(rng);
    char *err = randompack_last_error(rng);
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
  char *names[] = { "system", "system-csprng" };
  for (int i = 0; i < (int)(sizeof names/sizeof names[0]); i++) {
    randompack_rng *rng = create_seeded_rng(names[i], 0);
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
  randompack_rng *rng = create_seeded_rng("park-miller", 123);
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
  randompack_rng *r1 = create_seeded_rng("park-miller", 42);
  randompack_rng *r2 = create_seeded_rng("pm", 42);
  randompack_rng *r3 = create_seeded_rng("pm", 43);
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

void TestCreate(void) {
  test_determinism();
  test_engine_aliases();
  test_bad_engine_name();
  test_null_engine_name();
  test_default_engine_matches_x256pp();
  test_system_engine();
  test_park_miller();
  test_park_miller_determinism();
#ifndef HAVE128
  test_pcg64_unavailable();
#endif
}
