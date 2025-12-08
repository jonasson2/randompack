// -*- C -*-
// Tests for helper/statistics utilities.
#include <stdbool.h>

#include "TestUtil.h"
#include "xCheck.h"

bool test_moments_4vec(void) {
  // x = {0,3,2,1}: mean = 1.5, var = 5/3, skew = 0, excess kurtosis = -573/160
  double x[] = {0.0, 3.0, 2.0, 1.0};
  int n = 4;

  double xbar = mean(x, n);
  double s2 = var(x, n, xbar);
  double g1 = skewness(x, n, xbar, s2);
  double g2 = kurtosis(x, n, xbar, s2);

  double xbar_exp = 1.5;
  double s2_exp = 1.25;
  double g1_exp = 0.0;
  double g2_exp = 1.64;

  if (!almostSame(xbar, xbar_exp)) return false;
  if (!almostSame(s2,   s2_exp))   return false;
  if (!almostSame(g1,   g1_exp))   return false;
  if (!almostSame(g2,   g2_exp))   return false;

  return true;
}

static void test_moments(void) {
  xCheck(test_moments_4vec());
}

void TestHelpers(void) {
  test_moments();
}
