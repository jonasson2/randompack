#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <float.h>
#include "randompack.h"
#include "randompack_config.h"
#include "MatrixTestUtil.h"
#include "xCheck.h"
#include "printX.h"

// Check that counts is consistent with counts in bins with equal probability
// Use that Phi^{-1}(1 - r) ≈ sqrt(2·ln(1/r)) for small r
bool check_balanced_counts(int *counts, int n) {
  double q = 1e-13; // astromically low
  double mean, std, z, r, N = 0;
  for (int i=0; i<n; i++) N += counts[i];
  mean = (double)N/n;
  std = sqrt((double)N*(n - 1)/(n*n)); // of each count
  r = q/n/2;  // Bonferoni + each tail
  z = sqrt(2*log(1/r)); // max standardized deviation
  printD("counts allowed deviation", std*z);
  printIVS("counts", counts, n);
  for (int i=0; i<n; i++)
    if (fabs(counts[i] - mean) > std*z) return false;
  return true;
}

// Check that each bit is set with approx 50% probability
bool check_balanced_bits(int *ones, int N, int B) {
  double q = 1e-13; // astronomically low
  double r, mean, std, z;
  mean = (double)N/2;
  std = sqrt((double)N/4);
  r = q/B/2;  // Bonferoni + each tail
  z = sqrt(2*log(1/r)); // Max standardized deviation
  printD("bit allowed deviation", std*z);
  printIVS("bit-ones", ones, B);
  for (int b=0; b<B; b++)
    if (fabs(ones[b] - mean) > std*z) return false;
  return true;
}

// Check that the min and max of n U01 draws are statistically correct. It is easy to
// derive that an approximate (but higly accurate in this case) (1-q) confidence interval
// for xmin is [q/(2n), ln(2/q)/N], and for symmetry reason also for 1 - xmax.
static inline bool check_u01_minmax(double *x, int n) {
  if (n <= 0) return true; // vacuously OK
  const double q = 1e-13, lo, hi, xmin, xmax;
  lo = q/2/n;
  hi = ln(2/q)/n;
  xmin = minvd(x, n);
  xmax = maxvd(x, n);
  if (xmin < lo || xmin > hi) return false;
  if (1 - xmax < lo || 1 - xmax > hi) return false;
  return true;
}

bool equal_vec(int *a, int *b, int n) {
  for (int i = 0; i < n; i++) if (a[i] != b[i]) return false;
  return true;
}

bool equal_vecd(double *a, double *b, int n) {
  for (int i = 0; i < n; i++) if (a[i] != b[i]) return false;
  return true;
}

bool equal_vec32(uint32_t *a, uint32_t *b, int n) {
  for (int i = 0; i < n; i++) if (a[i] != b[i]) return false;
  return true;
}

bool equal_vec64(uint64_t *a, uint64_t *b, int n) {
  for (int i = 0; i < n; i++) if (a[i] != b[i]) return false;
  return true;
}

bool everywhere_different(uint64_t *a, uint64_t *b, int n) {
  for (int i = 0; i < n; i++) if (a[i] == b[i]) return false;
  return true;
}

double mean(double *x, int n) {
  if (n <= 0) return 0.0;
  double s = 0.0;
  for (int i = 0; i < n; i++) s += x[i];
  return s/n;
}

double var(double *x, int n, double mu) {
  if (n <= 1) return 0.0;
  double s = 0.0;
  for (int i = 0; i < n; i++) {
    double d = x[i] - mu;
    s += d*d;
  }
  return s/(n - 1);
}

int almostSame(double a, double b) {
  return relabsdiff(&a, &b, 1) < 5.0e-14;
}

int almostZero(double a[], int n) {
  int ia = iamax(n, a, 1);
  return fabs(a[ia]) < 5.0e-14;
}

int almostEqual(double a[], double b[], int n) {
  return relabsdiff(a, b, n) < 5.0e-14;
}

int almostAllSame(double a[], int n) {
  double minel, maxel, amax;
  if (n == 0) return 1;
  minel = maxel = a[0];
  for (int i = 1; i < n; i++) {
    minel = fmin(minel, a[i]);
    maxel = fmax(maxel, a[i]);
  }
  amax = fmax(fabs(minel), fabs(maxel));
  return (maxel - minel) <= 5.0e-14*fmax(1.0, amax);
}

void cov(char *transp, int m, int n, double X[], double C[]) {
  double *mu, *Xm;
  int i, tmp;
  if (transp[0] == 'T') { tmp = n; n = m; m = tmp; }
  setzero(n*n, C);
  if (m <= 1) return;
  ALLOC(mu, n);
  ALLOC(Xm, m*n);
  setzero(n, mu);
  if (transp[0] == 'T') copytranspose(n, m, X, n, Xm, m);
  else copy(m*n, X, 1, Xm, 1);
  for (i = 0; i < m; i++) axpy(n, 1.0, Xm + i, m, mu, 1);
  scal(n, 1.0/m, mu, 1);
  for (i = 0; i < m; i++) axpy(n, -1.0, mu, 1, Xm + i, m);
  syrk("Low", "T", n, m, 1.0/(m - 1), Xm, m, 0.0, C, n);
  copylowertoupper(n, C, n);
  FREE(Xm);
  FREE(mu);
}

uint32_t max32(uint32_t a, uint32_t b) {
  return a > b ? a : b;
}

uint64_t max64(uint64_t a, uint64_t b) {
  return a > b ? a : b;
}

int maxv(int *x, int n) {
  int m = INT_MIN;
  for (int i = 0; i < n; i++) m = max(m, x[i]);
  return m;
}

uint32_t maxv32(uint32_t *x, int n) {
  uint32_t m = 0;
  for (int i = 0; i < n; i++) m = max32(m, x[i]);
  return m;
}

uint64_t maxv64(uint64_t *x, int n) {
  uint64_t m = 0;
  for (int i = 0; i < n; i++) m = max64(m, x[i]);
  return m;
}

double maxvd(double *x, int n) {
  double m = -DBL_MAX;
  for (int i = 0; i < n; i++) m = fmax(m, x[i]);
  return m;
}

int minv(int *x, int n) {
  int m = INT_MAX;
  for (int i = 0; i < n; i++) m = min(m, x[i]);
  return m;
}

double minvd(double *x, int n) {
  double m = DBL_MAX;
  for (int i = 0; i < n; i++) m = fmin(m, x[i]);
  return m;
}

bool is_perm_0_to_n_minus1(int *x, int n) {
  int seen[n];
  for (int i = 0; i < n; i++) seen[i] = 0;
  for (int i = 0; i < n; i++) {
    int v = x[i];
    if (v < 0 || v >= n) return false;
    if (seen[v]) return false;
    seen[v] = 1;
  }
  return true;
}

// Verify that an RNG object is non-null and last_error is empty.
void check_rng_clean(randompack_rng *rng) {
  xCheck(rng);
  char *err = randompack_last_error(rng);
  xCheck(!err || !err[0]);
}

// Verify that an RNG call succeeded and last_error is empty.
void check_success(bool ok, randompack_rng *rng) {
  xCheck(ok);
  char *err = randompack_last_error(rng);
  xCheck(!err || !err[0]);
}

// Verify that an RNG call failed and produced a non-empty error.
void check_failure(bool ok, randompack_rng *rng) {
  xCheck(!ok);
  char *err = randompack_last_error(rng);
  xCheck(err && err[0]);
}
