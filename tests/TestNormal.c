// -*- C -*-
// Basic tests for randompack_norm: determinism and edge cases.
#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "xCheck.h"

// Helper: create an RNG and fill n normals.
static void draw_randoms(char *engine, double *x, int n, int seed) {
  randompack_rng *rng = randompack_create(engine, seed);
  check_rng_clean(rng);
  bool ok = randompack_norm(x, n, rng);
  check_success(ok, rng);
  randompack_free(rng);
}

// Different seeds => different randoms, same seeds => same randoms
static void test_determinism(char *engine) {
  double a[3], b[3], c[3];
  draw_randoms(engine, a, LEN(a), 42);
  draw_randoms(engine, b, LEN(b), 42);
  draw_randoms(engine, c, LEN(c), 43);
  xCheckMsg(equal_vecd(a, b, LEN(a)), engine);
  xCheckMsg(a[0] != c[0] || a[1] != c[1] || a[2] != c[2], engine);
}

// Edge cases: zero-length, null buffer/rng.
static void test_edge_cases(char *engine) {
  double buf[4] = {0.1, 0.2, 0.3, 0.4};
  double orig[4] = {0.1, 0.2, 0.3, 0.4};
  bool ok;
  randompack_rng *rng = randompack_create(engine, 123);
  ok = randompack_norm(buf, 0, rng);      check_success(ok, rng); // len = 0
  xCheck(equal_vecd(buf, orig, 4));                               // untouched buffer
  ok = randompack_norm(0, 4, rng);        check_failure(ok, rng); // null buffer w/len > 0
  ok = randompack_norm(buf, LEN(buf), 0); xCheck(!ok);            // null rng
  randompack_free(rng);
}

// Check sample mean and variance are near N(0,1) targets
static void test_statistics(char *engine) {
  int N = N_statistics;
  double *x, stdmu, stds2, mu, s2;
  ALLOC(x, N);
  draw_randoms(engine, x, N, 7);
  mu = 0;
  s2 = 1;
  stdmu = 1/sqrt((double)N);
  stds2 = sqrt(2.0/N);
  xCheck(check_meanvar(x, N, mu, s2, stdmu, stds2));
  FREE(x);
}

void TestNormal(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_determinism(e);
    test_edge_cases(e);
    test_statistics(e);
  }
}
