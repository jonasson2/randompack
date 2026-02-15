// -*- C -*-
// Tests for openlibm replacement functions.
#include <math.h>
#include "TestUtil.h"
#include "xCheck.h"
#include "randompack_internal.h"
#include "openlibm.inc"

static bool near(double a, double b, double tol) {
  double diff = fabs(a - b);
  double scale = fmax(1, fmax(fabs(a), fabs(b)));
  return diff <= tol*scale;
}

static void test_exp(void) {
  double x[] = {-20, -10, -5, -2, -1, -0.1, 0, 0.1, 1, 2, 5, 10};
  double tol = 1e-12;
  for (int i = 0; i < LEN(x); i++) {
    double a = openlibm_exp(x[i]);
    double b = exp(x[i]);
    xCheck(near(a, b, tol));
  }
}

static void test_log(void) {
  double x[] = {1e-300, 1e-100, 1e-10, 0.1, 0.5, 1, 2, 10, 1e10};
  double tol = 1e-12;
  for (int i = 0; i < LEN(x); i++) {
    double a = openlibm_log(x[i]);
    double b = log(x[i]);
    xCheck(near(a, b, tol));
  }
}

static void test_log1p(void) {
  double x[] = {1e-12, 1e-8, 1e-4, 0.1, 0.5, 0.9, 0.99, 0.999999, 1 - 1e-12};
  double tol = 1e-12;
  for (int i = 0; i < LEN(x); i++) {
    double a = openlibm_log1p(-x[i]);
    double b = log(1 - x[i]);
    xCheck(near(a, b, tol));
  }
}

void TestOpenlibm(void) {
  test_exp();
  test_log();
  test_log1p();
}
