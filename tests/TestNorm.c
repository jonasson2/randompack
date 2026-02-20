// -*- C -*-
// Tests for randompack_norm, N(0,1): max-value sanity checks.

#include <math.h>
#include <stdbool.h>
#include <string.h>

#include "test_util.h"
#include "test_cdfs.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

// Helper: check max absolute value drawn:
bool check_normal_max(double *x, int n) {
  double q, zlo, zhi, M;
  q = TEST_P_VALUE;
  zlo = probit((1 + pow(q/2, 1.0/n))/2);
  zhi = -probit(q/4/n);
  M = fmax(maxvd(x, n), -minvd(x, n));
  printD("normal max observed", M);
  printD("  lower confidence bound", zlo);
  printD("  upper confidence bound", zhi);
  return (zlo <= M && M <= zhi);
}

static void test_max(char *engine) {
  int N = N_STAT_FAST;
  double *x;
  float *y;
  TEST_ALLOC(x, N);
  TEST_ALLOC(y, N);
  DRAW(engine, 7, randompack_norm(x, N, rng));
  DRAW(engine, 7, randompack_normf(y, N, rng));
  xCheckMsg2(check_normal_max(x, N), engine, "double");
  for (int i = 0; i < N; i++) x[i] = y[i];
  xCheckMsg2(check_normal_max(x, N), engine, "float");
  FREE(y);
  FREE(x);
}

static void test_PIT(char *engine, int N) {
  double *x, *u;
  float *y, *v;
  TEST_ALLOC(x, N);
  TEST_ALLOC(u, N);
  TEST_ALLOC(y, N);
  TEST_ALLOC(v, N);
  DRAW(engine, 42, randompack_norm(x, N, rng));
  DRAW(engine, 42, randompack_normf(y, N, rng));
  for (int i = 0; i < N; i++) u[i] = test_cdf_norm(x[i]);
  for (int i = 0; i < N; i++) v[i] = (float)test_cdf_norm((double)y[i]);
  check_u01_distribution(u, N, "norm", engine);
  check_u01_distributionf(v, N, "norm", engine);
  FREE(v);
  FREE(y);
  FREE(u);
  FREE(x);
}

static void test_edge_cases(char *engine) {
  double buf[4] = { 1, 2, 3, 4 };
  double orig[4] = { 1, 2, 3, 4 };
  float buf_f[4] = { 1, 2, 3, 4 };
  float orig_f[4] = { 1, 2, 3, 4 };
  bool ok;
  randompack_rng *rng = create_seeded_rng(engine, 123);
  memcpy(buf, orig, sizeof(buf));
  memcpy(buf_f, orig_f, sizeof(buf_f));
  ok = randompack_norm(buf, 0, rng); check_success(ok, rng);
  CHECK_VEC_EQUAL(double, buf, orig, LEN(buf));
  ok = randompack_normf(buf_f, 0, rng); check_success(ok, rng);
  CHECK_VEC_EQUAL(float, buf_f, orig_f, LEN(buf_f));
  ok = randompack_norm(0, LEN(buf), rng); check_failure(ok, rng);
  ok = randompack_normf(0, LEN(buf_f), rng); check_failure(ok, rng);
  ok = randompack_norm(buf, LEN(buf), 0); xCheck(!ok);
  ok = randompack_normf(buf_f, LEN(buf_f), 0); xCheck(!ok);
  randompack_free(rng);
}

static void test_determinism(char *engine) {
  double xd[3], yd[3], zd[3];
  float xf[3], yf[3], zf[3];
  int len = LEN(xd);
  DRAW(engine, 42, randompack_norm(xd, len, rng));
  DRAW(engine, 42, randompack_norm(yd, len, rng));
  DRAW(engine, 43, randompack_norm(zd, len, rng));
  CHECK_VEC_EQUAL(double, xd, yd, len);
  CHECK_VEC_DIFFERENT(double, xd, zd, len);
  DRAW(engine, 42, randompack_normf(xf, len, rng));
  DRAW(engine, 42, randompack_normf(yf, len, rng));
  DRAW(engine, 43, randompack_normf(zf, len, rng));
  CHECK_VEC_EQUAL(float, xf, yf, len);
  CHECK_VEC_DIFFERENT(float, xf, zf, len);
}

void TestNorm(void) {
  int n = 0;
  char **engines = get_engines(&n);
  for (int i = 0; i < n; i++) {
    char *e = engines[i];
    test_edge_cases(e);
    test_determinism(e);
    test_max(e);
    test_PIT(e, N_STAT_SLOW);
  }
  free_engines(engines, n);
}

void TestNormx(char *engine) {
  char *e = engine;
  test_edge_cases(e);
  test_determinism(e);
  test_max(e);
  test_PIT(e, N_STAT_FAST);
}
