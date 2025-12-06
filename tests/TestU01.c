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

// Different seeds => different randoms, same seeds => same randoms
static void test_determinism(char *engine) {
  double a[3], b[3], c[3];
  draw_randoms(engine, a, LEN(a), 42);
  draw_randoms(engine, b, LEN(b), 42);
  draw_randoms(engine, b, LEN(c), 43);
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
static void test_meanvar(char *engine) {
  int N = 1e5;
  double stdmu = 1/sqrt(12*N);
  double stdvar = 1/sqrt(180*N);
  double meantol = 7*stdmu; // astronomically unlikely to exceed
  double vartol = 7*stdvar; // ditto
  double exactmu = 0.5, exactvar = 1.0/12.0;
  double *x;
  ALLOC(x, N);
  draw_randoms(engine, x, N, 42);
  xCheck(0 <= minvd(x, N) && maxvd(x, N) <= 1); 
  double mu = mean(x, N);
  double va = var(x, N, mu);
  xCheck(fabs(mu - exactmu) < meantol);
  xCheck(fabs(va - exactvar) < vartol);
  int counts[20] = {0};
  for (int i = 0; i < N; i++) {
    double v = x[i];
    int b = (int)(v*20.0);
    if (b >= 20) b = 19;
    xCheck(0 <= b && b < 20);
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
    test_meanvar(e);
  }
}
