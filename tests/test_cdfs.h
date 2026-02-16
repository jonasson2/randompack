// Utilities for CDFs in tests
#ifndef TEST_CDF_UTIL_H
#define TEST_CDF_UTIL_H

double test_cdf_t(double x, double nu);
double test_cdf_gumbel(double x, double mu, double beta);
double test_cdf_pareto(double x, double xm, double alpha);
double test_cdf_weibull(double x, double shape, double scale);
double test_cdf_f(double x, double nu1, double nu2);
double test_cdf_chi2(double x, double nu);
double test_cdf_skew_normal(double x, double mu, double sigma, double alpha);
double test_cdf_lognormal(double x, double mu, double sigma);
double test_cdf_norm(double x);
double test_cdf_normal(double x, double mu, double sigma);
double test_cdf_exp(double x, double scale);
double test_cdf_beta(double x, double a, double b);

#endif
