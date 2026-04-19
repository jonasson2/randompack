// Utilities for CDFs in tests
#include <math.h>
#include "test_cdfs.h"
#include "test_util.h"
#include "cephes.h"

double test_cdf_t(double x, double nu) {
  double z = nu/(nu + x*x);
  double p = 0.5*incbet(0.5*nu, 0.5, z);
  return (x > 0) ? 1 - p : p;
}

double test_cdf_gumbel(double x, double mu, double beta) {
  return exp(-exp(-(x - mu)/beta));
}

double test_cdf_pareto(double x, double xm, double alpha) {
  if (x < xm) return 0;
  return 1 - pow(xm/x, alpha);
}

double test_cdf_weibull(double x, double shape, double scale) {
  if (x < 0) return 0;
  return 1 - exp(-pow(x/scale, shape));
}

double test_cdf_f(double x, double nu1, double nu2) {
  if (x <= 0) return 0;
  if (!isfinite(x)) return 1;
  double z = (nu1*x)/(nu1*x + nu2);
  return incbet(0.5*nu1, 0.5*nu2, z);
}

double test_cdf_chi2(double x, double nu) {
  return chi2_cdf(x, (int)nu);
}

double test_cdf_skew_normal(double x, double mu, double sigma, double alpha) {
  (void)alpha;
  return normcdf((x - mu)/sigma);
}

double test_cdf_lognormal(double x, double mu, double sigma) {
  if (x <= 0) return 0;
  return normcdf((log(x) - mu)/sigma);
}

double test_cdf_norm(double x) {
  return normcdf(x);
}

double test_cdf_normal(double x, double mu, double sigma) {
  return normcdf((x - mu)/sigma);
}

double test_cdf_exp(double x, double scale) {
  if (x <= 0) return 0;
  return 1 - exp(-x/scale);
}

double test_cdf_beta(double x, double a, double b) {
  if (x <= 0) return 0;
  if (x >= 1) return 1;
  return incbet(a, b, x);
}
