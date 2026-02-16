// -*- C -*-
// Tests for Sleef log/exp in-place helpers.
#include <math.h>
#include "test_util.h"
#include "randompack_internal.h"
#include "openlibm.inc"
#include "xCheck.h"
#include "log_exp.inc"

void TestSleefMath(void) {
  int n = 100;
  size_t len = (size_t)n;
  randompack_rng *rng = create_seeded_rng("x256++simd", 123);
  bool ok = randompack_fast_logexp(rng, true);
  check_success(ok, rng);
  double *x, *y, *z, *ref;
  float *xf, *yf, *zf, *reff;
  TEST_ALLOC(x, n);
  TEST_ALLOC(y, n);
  TEST_ALLOC(z, n);
  TEST_ALLOC(ref, n);
  TEST_ALLOC(xf, n);
  TEST_ALLOC(yf, n);
  TEST_ALLOC(zf, n);
  TEST_ALLOC(reff, n);
  ok = randompack_u01(x, len, rng);
  check_success(ok, rng);
  ok = randompack_u01f(xf, len, rng);
  check_success(ok, rng);
  for (int i = 0; i < n; i++) {
    y[i] = x[i];
    z[i] = x[i];
    yf[i] = xf[i];
    zf[i] = xf[i];
  }
  log_inplace(y, len, rng);
  for (int i = 0; i < n; i++) ref[i] = log(x[i]);
  xCheck(almostEqual(y, ref, n));
  logf_inplace(yf, len, rng);
  for (int i = 0; i < n; i++) reff[i] = logf(xf[i]);
  xCheck(almostEqualf(yf, reff, n));
  exp_inplace(z, len, rng);
  for (int i = 0; i < n; i++) ref[i] = exp(x[i]);
  xCheck(almostEqual(z, ref, n));
  expf_inplace(zf, len, rng);
  for (int i = 0; i < n; i++) reff[i] = expf(xf[i]);
  xCheck(almostEqualf(zf, reff, n));
  FREE(x);
  FREE(y);
  FREE(z);
  FREE(ref);
  FREE(xf);
  FREE(yf);
  FREE(zf);
  FREE(reff);
  randompack_free(rng);
}
