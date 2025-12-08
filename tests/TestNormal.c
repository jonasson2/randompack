// -*- C -*-
// Basic tests for randompack_norm: determinism and edge cases.
#include <math.h>
#include <stdbool.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"

// Helper: create an RNG and fill n normals.
static void draw_randoms(char *engine, char *method, double *x, int n,int seed) {
  randompack_rng *rng = randompack_create(engine, seed);
  check_rng_clean(rng);
  bool ok = randompack_set_norm_method(method, rng);
  check_success(ok, rng);
  ok = randompack_norm(x, n, rng);
  check_success(ok, rng);
  randompack_free(rng);
}

// Helper: check max absolute value drawn:
bool check_normal_max(double *x, int n) {
  // [zlo, zhi] is the (1-q) confidence interval for max|x|; fairly easy to derive
  double q, zlo, zhi, M;
  q = TEST_P_VALUE;
  zlo = probit((1 + pow(q/2, 1.0/n))/2);  // accurate
  zhi = -probit(q/4/n);  // a very accurate approximation for q < 0.01, n > 100
  M = fmax(maxvd(x, n), -minvd(x, n));
  printD("normal max observed", M);
  printD("  lower confidence bound", zlo);
  printD("  upper confidence bound", zhi);
  return (zlo <= M && M <= zhi);
}

// Different seeds => different randoms, same seeds => same randoms
static void test_determinism(char *engine, char *method) {
  double a[3], b[3], c[3];
  draw_randoms(engine, method, a, LEN(a), 42);
  draw_randoms(engine, method, b, LEN(b), 42);
  draw_randoms(engine, method, c, LEN(c), 43);
  xCheckMsg(equal_vecd(a, b, LEN(a)), engine);
  xCheckMsg(a[0] != c[0] || a[1] != c[1] || a[2] != c[2], engine);
}

// Edge cases: zero-length, null buffer/rng.
static void test_edge_cases(char *engine, char *method) {
  double buf[4] = {0.1, 0.2, 0.3, 0.4};
  double orig[4] = {0.1, 0.2, 0.3, 0.4};
  bool ok;
  randompack_rng *rng = randompack_create(engine, 123);
  check_rng_clean(rng);
  ok = randompack_set_norm_method(method, rng);
  check_success(ok, rng);
  ok = randompack_norm(buf, 0, rng);      check_success(ok, rng); // len = 0
  xCheck(equal_vecd(buf, orig, 4));                               // untouched buffer
  ok = randompack_norm(0, 4, rng);        check_failure(ok, rng); // null buffer w/len > 0
  randompack_free(rng);
}

// Check that sample mean, variance, skewness, and kurtosis agree with theoretical values
static void test_statistics(char *engine, char *method) {
  int N = 1000;//N_statistics;
  double *x, mu, xbar, s2, stdmu, sigma2, stdsigma2, skew, skewstd, kurt, kurtstd;
  ALLOC(x, N);
  draw_randoms(engine, method, x, N, 7);
  //
  // Check mean and variance
  mu = 0;
  sigma2 = 1;
  stdmu = sqrt(1.0/N);
  stdsigma2 = sqrt(2.0/N);
  xCheck(check_meanvar(x, N, mu, sigma2, stdmu, stdsigma2));
  //
  // Check skewness and kurtosis
  xbar = mean(x, N);
  s2 = var(x, N, xbar);
  skew = 0;
  kurt = 3;
  skewstd = sqrt(6.0/N);
  kurtstd = sqrt(24.0/N);
  xCheck(check_skew(x, N, xbar, s2, skew, skewstd));
  xCheck(check_kurt(x, N, xbar, s2, kurt, kurtstd));
  // 
  // Check maximum (and minimum)
  xCheck(check_normal_max(x, N));
  //
  FREE(x);
}

static char *methods[] = {"default", "polar"};

// Basic check of selecting method
static void test_method_selection(char *engine) {
  randompack_rng *rng = randompack_create(engine, 42);
  check_rng_clean(rng);
  bool ok = randompack_set_norm_method("garbage", rng);
  check_failure(ok, rng);
  randompack_free(rng);
  for (int m = 0; m < 2; m++) {
    randompack_rng *rng = randompack_create(engine, 42);
    check_rng_clean(rng);
    ok = randompack_set_norm_method(methods[m], rng);
    check_success(ok, rng);
    randompack_free(rng);
  }
}

// Call all the test functions
void TestNormal(void) {
  for (int i = 0; i < LEN(engines); i++) {
	 char *e = engines[i];
	 test_method_selection(e);
	 for (int k = 0; k < 2; k++) {
		char *m = methods[k];	 
		test_determinism(e, m);
		test_edge_cases(e, m);
		test_statistics(e, m);
	 }
  }
}
