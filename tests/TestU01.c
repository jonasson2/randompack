// -*- C -*-
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include "TestUtil.h"
#include "randompack_config.h"
#include "printX.h"
#include "randompack.h"
#include "xCheck.h"

// Helper: create an RNG and fill n doubles in [0,1).
static void draw_randoms(char *engine, double *x, int n, int seed) {
  randompack_rng *rng = randompack_create(engine, seed);
  check_rng_clean(rng);
  bool ok = randompack_u01(x, n, rng);
  check_success(ok, rng);
  randompack_free(rng);
}

// Helper: Check min and max
bool check_u01_minmax(double *x, int n) {
  if (n <= 0) return true;
  const double q = 1e-13;
  double lo = q/2/n;      // ≈ Betainv(q/2, 1, n)
  double hi = log(2/q)/n; // ≈ Betainv(1-q/2, n, 1)
  double xmin = minvd(x, n);
  double xmax = maxvd(x, n);
  if (xmin < lo || xmin > hi) return false;
  if (1 - xmax < lo || 1 - xmax > hi) return false;
  return true;
}

// Different seeds => different randoms, same seeds => same randoms
static void test_determinism(char *engine) {
  double a[3], b[3], c[3];
  draw_randoms(engine, a, LEN(a), 42);
  draw_randoms(engine, b, LEN(b), 42);
  draw_randoms(engine, c, LEN(c), 43);
  xCheckMsg(equal_vecd(a, b, LEN(a)), engine);
  xCheckMsg(a[0] != c[0] && a[1] != c[1] && a[2] != c[2], engine);
}

// Edge cases: zero-length, null buffer/rng, and bad bounds, max span.
static void test_edge_cases(char *engine) {
  double buf[4] = {0.1, 0.2, 0.3};
  double orig[4] = {0.1, 0.2, 0.3};
  bool ok;
  randompack_rng *rng = randompack_create(engine, 123);
  ok = randompack_u01(buf, 0, rng);      check_success(ok, rng); // len = 0
  xCheck(equal_vecd(buf, orig, 4));                              // –doesn't touch buffer
  ok = randompack_u01(0, 4, rng);        check_failure(ok, rng); // null buffer w/len > 0
  ok = randompack_u01(buf, LEN(buf), 0); xCheck(!ok);            // null rng
  randompack_free(rng);
}

// Test the mean and variance of x are as expected
static void test_statistics(char *engine) {
  int N = N_statistics;
  double *x, stdmu, stds2, mu, s2;  

  // Draw a sample
  ALLOC(x, N);
  draw_randoms(engine, x, N, 42);

  // Check mean and variance
  mu = 0.5;
  s2 = 1.0/12.0;
  stdmu = 1/sqrt(12*N);
  stds2 = 1/sqrt(180*N);
  xCheck(check_meanvar(x, N, mu, s2, stdmu, stds2));

  // Check xmin and xmax (also checks that all are in [0,1])
  check_u01_minmax(x, N);

  // Check that bin counts are balanced
  int counts[20] = {0};
  for (int i = 0; i < N; i++) {
    int b = min(19, (int)(20*x[i]));
    counts[b]++;
  }
  xCheck(check_balanced_counts(counts, 20));
  FREE(x);
}

void TestU01(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_determinism(e);
    test_edge_cases(e);
    test_statistics(e);
  }
}
