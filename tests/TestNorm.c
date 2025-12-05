// -*- C -*-
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "allocate.h"
#include "ExtraUtil.h"
#include "printX.h"
#include "randompack.h"
#include "xCheck.h"

static void test_normal_basic(void) {
  const int N = 1e6;
  double meantol = 7*1/sqrt(N);
  double vartol = 7*sqrt(2.0/(N-1));
  double exactmu = 0.0, exactvar = 1.0;
  double *x = 0;
  allocate(x, N);

  randompack_rng *rng = randompack_create("Xorshift", 0);
  randompack_norm(x, N, rng);
  double mu = mean(x, N);
  double va = var(x, N, mu);
  printD("va", va);
  printD("exactvar", exactvar);
  printD("vartol", vartol);
  xCheck(fabs(mu - exactmu) < meantol);
  xCheck(fabs(va - exactvar) < vartol);

  randompack_free(rng);
  freem(x);
}

static void test_determinism_default_seed(void) {
  const int N = 1000;
  double *a = 0, *b = 0;
  allocate(a, N);
  allocate(b, N);

  randompack_rng *r1 = randompack_create("Xorshift", 123);
  randompack_rng *r2 = randompack_create("Xorshift", 123);

  randompack_norm(a, N, r1);
  randompack_norm(b, N, r2);

  xCheck(almostEqual(a, b, N));

  randompack_free(r1);
  randompack_free(r2);
  freem(a);
  freem(b);
}

void TestNorm(void) {
  test_normal_basic();
  test_determinism_default_seed();
}
