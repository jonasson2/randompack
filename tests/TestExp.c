// -*- C -*-
// Basic tests for randompack_exp: determinism and simple stats (mean/variance).

#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

// Helper: create an RNG and fill n exponentials.
static void draw_randoms(char *engine, double *x, int n, int seed) {
  randompack_rng *rng = create_seeded_rng(engine, seed);
  check_rng_clean(rng);
  bool ok = randompack_exp(x, n, rng);     // <-- rename if your API differs
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

// Mean/variance check. Standard exponential has mean=1, var=1.
// Use a generous k-sigma bound (e.g. 8) to keep the test stable.
static void test_meanvar(char *engine) {
  enum { N = 20000 };
  double *x;
  TEST_ALLOC(x, N);

  draw_randoms(engine, x, N, 7);

  const double mu = 1.0;
  const double sigma2 = 1.0;

  // For Exp(1): Var(mean)=1/N, and Var(sample var) ~ 8/N (large-sample approx),
  // so std(var) ~ sqrt(8/N). This is a common quick check.
  const double k = 8.0;
  const double stdmu = k * sqrt(1.0 / N);
  const double stdsigma2 = k * sqrt(8.0 / N);

  xCheck(check_meanvar(x, N, mu, sigma2, stdmu, stdsigma2));

  FREE(x);
}

void TestExp(void) {
  for (int i = 0; i < LEN(engines); i++) {
    char *e = engines[i];
    test_determinism(e);
    test_meanvar(e);
  }
}
