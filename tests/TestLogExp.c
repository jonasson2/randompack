// -*- C -*-
// Tests for fast_logexp flag
#include <limits.h>
#include <math.h>
#include "TestUtil.h"
#include "xCheck.h"
#include "randompack_internal.h"
#include "openlibm.inc"
#include "log_exp.inc"

static bool near(double a, double b, double tol) {
  double diff = fabs(a - b);
  double scale = fmax(1, fmax(fabs(a), fabs(b)));
  return diff <= tol*scale;
}

static void check_unif(randompack_rng *rng, char *engine) {
  int n = N_STAT_SLOW;
  double *x, *u;
  TEST_ALLOC(x, n);
  TEST_ALLOC(u, n);
  bool ok = randompack_unif(x, n, 0, 1, rng);
  check_success(ok, rng);
  for (int i = 0; i < n; i++) u[i] = x[i];
  check_u01_distribution(u, n, "unif(0,1)", engine);
  FREE(x);
  FREE(u);
}

static void check_exp2(randompack_rng *rng, char *engine) {
  int n = N_STAT_SLOW;
  double *x, *u;
  TEST_ALLOC(x, n);
  TEST_ALLOC(u, n);
  bool ok = randompack_exp(x, n, 2, rng);
  check_success(ok, rng);
  for (int i = 0; i < n; i++) {
    double xi = x[i];
    u[i] = xi <= 0 ? 0 : 1 - exp(-xi/2);
  }
  check_u01_distribution(u, n, "exp(2)", engine);
  FREE(x);
  FREE(u);
}

static void check_lognormal(randompack_rng *rng, char *engine) {
  int n = N_STAT_SLOW;
  double *x, *u;
  TEST_ALLOC(x, n);
  TEST_ALLOC(u, n);
  bool ok = randompack_lognormal(x, n, 2, 3, rng);
  check_success(ok, rng);
  for (int i = 0; i < n; i++) {
    double xi = x[i];
    if (xi <= 0) u[i] = 0;
    else u[i] = normcdf((log(xi) - 2)/3);
  }
  check_u01_distribution(u, n, "lognormal(2,3)", engine);
  FREE(x);
  FREE(u);
}

static void check_weibull(randompack_rng *rng, char *engine) {
  int n = N_STAT_SLOW;
  double *x, *u;
  TEST_ALLOC(x, n);
  TEST_ALLOC(u, n);
  bool ok = randompack_weibull(x, n, 2, 3, rng);
  check_success(ok, rng);
  for (int i = 0; i < n; i++) {
    double xi = x[i];
    if (xi <= 0) u[i] = 0;
    else u[i] = 1 - exp(-pow(xi/3, 2));
  }
  check_u01_distribution(u, n, "weibull(2,3)", engine);
  FREE(x);
  FREE(u);
}

static void test_sleef_math(randompack_rng *rng) {
  int n = 100;
  double *x, *y, *z;
  TEST_ALLOC(x, n);
  TEST_ALLOC(y, n);
  TEST_ALLOC(z, n);
  bool ok = randompack_u01(x, n, rng);
  check_success(ok, rng);
  for (int i = 0; i < n; i++) {
    y[i] = x[i];
    z[i] = x[i];
  }
  log_inplace(y, n, rng);
  for (int i = 0; i < n; i++) {
    double ref = log(x[i]);
    if (!isfinite(ref)) xCheck(!isfinite(y[i]));
    else xCheck(near(y[i], ref, 1e-10));
  }
  exp_inplace(z, n, rng);
  for (int i = 0; i < n; i++) {
    double ref = exp(x[i]);
    if (!isfinite(ref)) xCheck(!isfinite(z[i]));
    else xCheck(near(z[i], ref, 1e-10));
  }
  FREE(x);
  FREE(y);
  FREE(z);
}

void TestLogExp(void) {
  char *engines[] = {"x256++simd", "pcg64"};
  for (int i = 0; i < LEN(engines); i++) {
    char *engine = engines[i];
    randompack_rng *rng = create_seeded_rng(engine, 123);
    bool ok = randompack_fast_logexp(rng, true);
    check_success(ok, rng);
    check_unif(rng, engine);
    check_exp2(rng, engine);
    check_lognormal(rng, engine);
    check_weibull(rng, engine);
    test_sleef_math(rng);
    randompack_free(rng);
  }
}
