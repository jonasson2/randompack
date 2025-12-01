// Utilities used by randompack tests
#include "VarmaUtilities.h"
#include "allocate.h"
#include "ExtraUtil.h"
#include "BlasGateway.h"
#include "error.h"
#include <math.h>
#include <stdio.h>

double mean(const double *x, int n) {
  // Mean of vector
  if (n <= 0) return 0.0;
  double s = 0.0; for (int i=0; i<n; i++) s += x[i];
  return s/n;
}

double var(const double *x, int n, double mu) {
  // Unbiased variance of vector
  if (n <= 1) return 0.0;
  double s = 0.0;
  for (int i=0; i<n; i++) { double d = x[i] - mu; s += d*d; }
  return s/(n - 1);
}

int almostSame(double a, double b) {
  // true if a and b are almost equal
  return relabsdiff(&a, &b, 1) < 5.0e-14;
}

int almostZero(double a[], int n) {
  // true if a ≈ 0
  int ia = iamax(n, a, 1);
  return fabs(a[ia]) < 5.0e-14;
}

int almostEqual(double a[], double b[], int n) {
  // true if relative difference between vectors a and b < 5e-14
  return relabsdiff(a, b, n) < 5.0e-14;
}

int almostAllSame(double a[], int n) {
  // true if max difference among elements of a is < 5e-14
  double minel, maxel, amax;
  int i;
  if (n == 0) return 1;
  minel = maxel = a[0];
  for (i=1; i<n; i++) {
    minel = fmin(minel, a[i]);
    maxel = fmax(maxel, a[i]);
  }
  amax = fmax(fabs(minel),fabs(maxel));
  return (maxel - minel) <= 5e-14*fmax(1,amax);
}

void cov(char *transp, int m, int n, double X[], double C[]) {
  // C := covariance between columns of the m by n matrix op(X). If transp begins with N,
  // op(X) = X, but if it begins with T, op(X) = X^T. C is n by n.
  double *mu, *Xm;
  int i, tmp;
  if (transp[0] == 'T') { tmp = n; n = m; m = tmp; }
  setzero(n*n, C);
  if (m <= 1) return;
  allocate(mu, n);
  allocate(Xm, m*n);
  setzero(n, mu);
  if (transp[0] == 'T') copytranspose(n, m, X, n, Xm, m);
  else copy(m*n, X, 1, Xm, 1);
  for (i=0; i<m; i++) axpy(n, 1.0, Xm + i, m, mu, 1);
  scal(n, 1.0/m, mu, 1); // mu[i] = mean of X(:,i)
  for (i=0; i<m; i++) axpy(n, -1.0, mu, 1, Xm + i, m); // subtract mu from rows
  syrk("Low", "T", n, m, 1.0/(m-1), Xm, m, 0.0, C, n);
  copylowertoupper(n, C, n);
  freem(Xm); freem(mu);
}
