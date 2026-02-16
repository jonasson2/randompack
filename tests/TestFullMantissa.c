// -*- C -*-
// Tests for full_mantissa flag
#include <math.h>
#include "TestUtil.h"
#include "xCheck.h"

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

void TestFullMantissa(void) {
  char *engines[] = {"x256++simd", "pcg64"};
  for (int i = 0; i < LEN(engines); i++) {
    char *engine = engines[i];
    randompack_rng *rng = create_seeded_rng(engine, 123);
    bool ok = randompack_full_mantissa(rng, true);
    check_success(ok, rng);
    check_unif(rng, engine);
    check_exp2(rng, engine);
    check_lognormal(rng, engine);
    check_weibull(rng, engine);
    randompack_free(rng);
  }
}
