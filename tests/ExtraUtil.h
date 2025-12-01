// ExtraUtils.h — Utilities used by randompack tests (means, covariances, approx equality)

#ifndef EXTRAUTIL_H
#define EXTRAUTIL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
double mean(const double *x, int n);               // Mean of vector
double var(const double *x, int n, double mu);     // Unbiased variance of vector
int almostSame(double a, double b);                // are a and b are almost equal?
int almostEqual(double a[], double b[], int n);    // is rel.diff. beween a and b < 5e-14?
int almostAllSame(double a[], int n);              // is max diff. among x-elements < 5e-14?
int almostZero(double a[], int n);                 // is a ≈ 0?

void cov(char *transp, int m, int n, double X[], double C[]);
// C := covariance between columns of op(X). X is an m by n matrix. If transp begins with
// N, op(X) = X, and C is n by n, but if it begins with T, op(X) = X^T and C is m by m.

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* EXTRAUTIL_H */
