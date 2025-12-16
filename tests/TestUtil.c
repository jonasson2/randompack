#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <float.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "MatrixTestUtil.h"
#include "xCheck.h"
#include "printX.h"

//------------------------------------------------------------------------------
// Vector equality / difference helpers
//------------------------------------------------------------------------------

bool equal_vec(int *a, int *b, int n) {
  for (int i = 0; i < n; i++) if (a[i] != b[i]) return false;
  return true;
}

bool equal_vecd(double *a, double *b, int n) {
  for (int i = 0; i < n; i++) if (a[i] != b[i]) return false;
  return true;
}

bool equal_vec8(uint8_t *a, uint8_t *b, int n) {
  for (int i = 0; i < n; i++) if (a[i] != b[i]) return false;
  return true;
}

bool equal_vec16(uint16_t *a, uint16_t *b, int n) {
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

//------------------------------------------------------------------------------
// Approximate equality
//------------------------------------------------------------------------------

int almostSame(double a, double b) {
  return relabsdiff(&a, &b, 1) < 5.0e-14;
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

int almostZero(double a[], int n) {
  int ia = iamax(n, a, 1);
  return fabs(a[ia]) < 5.0e-14;
}

//------------------------------------------------------------------------------
// Simple statistics
//------------------------------------------------------------------------------

double mean(double *x, int n) {
  if (n <= 0) return 0.0;
  double sum = 0.0;
  for (int i = 0; i < n; i++) sum += x[i];
  return sum/n;
}

double var(double *x, int n, double xbar) {
  if (n <= 1) return 0.0;
  double sum = 0.0;
  for (int i = 0; i < n; i++) {
    double d = x[i] - xbar;
    sum += d*d;
  }
  return sum/n;
}

double skewness(double *x, int n, double xbar, double s2) {
  if (n <= 2 || s2 <= 0.0) return 0.0;
  double m3 = 0.0;
  for (int i = 0; i < n; i++) {
    double d = x[i] - xbar;
    m3 += d*d*d;
  }
  return m3/(n*s2*sqrt(s2));
}

double kurtosis(double *x, int n, double xbar, double s2) {
  if (n <= 3 || s2 <= 0) return 0.0;
  double m4 = 0.0;
  for (int i = 0; i < n; i++) {
    double d = x[i] - xbar;
    m4 += d*d*d*d;
  }
  return m4/(n*s2*s2);
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

bool check_meanvar(double *x, int n, double mu, double sigma2, double std_mu,
						 double std_sigma2) {
  double xbar = mean(x, n);
  double s2 = var(x, n, xbar);
  double meantol = 7*std_mu;
  double vartol = 7*std_sigma2;
  if (fabs(xbar - mu) >= meantol) return false;
  if (fabs(s2 - sigma2) >= vartol) return false;
  return true;
}

bool check_skew(double *x, int n, double xbar, double s2, double skew, double skew_std) {
  double skew_hat;
  double tol = 7*skew_std;
  skew_hat = skewness(x, n, xbar, s2);
  printD("skew lower bound", skew - tol);
  printD("skew upper bound", skew + tol);
  printD("skew observed", skew_hat);
  if (fabs(skew_hat - skew) >= tol) return false;
  return true;
}

bool check_kurt(double *x, int n, double xbar, double s2, double kurt, double kurt_std) {
  double kurt_hat;
  double tol = 7*kurt_std;
  kurt_hat = kurtosis(x, n, xbar, s2);
  if (fabs(kurt_hat - kurt) >= tol) return false;
  return true;
}

// Inverse normal CDF. Translation of PPND16 in AS241 (Wichura, 1988), public domain.
#include <math.h>

double probit(double p) { // Return NAN if p <= 0 or p >= 1
  // Constants
  const double split1 = 0.425, split2 = 5.0, const1 = 0.180625, const2 = 1.6;
  const double
    // Coefficients for p close to 0.5
    a0 = 3.3871328727963666080e0, b1 = 4.2313330701600911252e1,
    a1 = 1.3314166789178437745e2, b2 = 6.8718700749205790830e2,
    a2 = 1.9715909503065514427e3, b3 = 5.3941960214247511077e3,
    a3 = 1.3731693765509461125e4, b4 = 2.1213794301586595867e4,
    a4 = 4.5921953931549871457e4, b5 = 3.9307895800092710610e4,
    a5 = 6.7265770927008700853e4, b6 = 2.8729085735721942674e4,
    a6 = 3.3430575583588128105e4, b7 = 5.2264952788528545610e3,
    a7 = 2.5090809287301226727e3,

    // Coefficients for p not close to 0, 0.5 or 1
    c0 = 1.42343711074968357734e0,  d1 = 2.05319162663775882187e0, 
    c1 = 4.63033784615654529590e0,  d2 = 1.67638483018380384940e0, 
    c2 = 5.76949722146069140550e0,  d3 = 6.89767334985100004550e-1,
    c3 = 3.64784832476320460504e0,  d4 = 1.48103976427480074590e-1,
    c4 = 1.27045825245236838258e0,  d5 = 1.51986665636164571966e-2,
    c5 = 2.41780725177450611770e-1, d6 = 5.47593808499534494600e-4,
    c6 = 2.27238449892691845833e-2, d7 = 1.05075007164441684324e-9,
    c7 = 7.74545014278341407640e-4,

    // Coefficients for p near 0 or 1
    e0 = 6.65790464350110377720e0,  f1 = 5.99832206555887937690e-1, 
    e1 = 5.46378491116411436990e0,  f2 = 1.36929880922735805310e-1, 
    e2 = 1.78482653991729133580e0,  f3 = 1.48753612908506148525e-2, 
    e3 = 2.96560571828504891230e-1, f4 = 7.86869131145613259100e-4, 
    e4 = 2.65321895265761230930e-2, f5 = 1.84631831751005468180e-5, 
    e5 = 1.24266094738807843860e-3, f6 = 1.42151175831644588870e-7, 
    e6 = 2.71155556874348757815e-5, f7 = 2.04426310338993978564e-15,
    e7 = 2.01033439929228813265e-7;
  
  double q, r, x;
  q = p - 0.5e0;
  if (fabs(q) <= split1) {
    r = const1 - q*q;
    x = q * (((((((a7*r + a6)*r + a5)*r + a4)*r + a3)*r + a2)*r + a1)*r + a0) /
             (((((((b7*r + b6)*r + b5)*r + b4)*r + b3)*r + b2)*r + b1)*r + 1.0);
    return x;
  }
  else {
    if (q < 0.0) r = p;
    else {
      r = 1.0 - p;
    }
    if (r <= 0.0) return NAN;
    r = sqrt(-log(r));
    if (r <= split2) {
      r = r - const2;
      x = (((((((c7*r + c6)*r + c5)*r + c4)*r + c3)*r + c2)*r + c1)*r + c0) /
          (((((((d7*r + d6)*r + d5)*r + d4)*r + d3)*r + d2)*r + d1)*r + 1.0);
    }
    else {
      r = r - split2;
      x = (((((((e7*r + e6)*r + e5)*r + e4)*r + e3)*r + e2)*r + e1)*r + e0) /
          (((((((f7*r + f6)*r + f5)*r + f4)*r + f3)*r + f2)*r + f1)*r + 1.0);
    }
    if (q < 0.0) x = -x;
    return x;
  }
}

//------------------------------------------------------------------------------
// Min/max helpers for scalars and vectors
//------------------------------------------------------------------------------

uint8_t max8(uint8_t a, uint8_t b) {
  return a > b ? a : b;
}

uint16_t max16(uint16_t a, uint16_t b) {
  return a > b ? a : b;
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

uint8_t maxv8(uint8_t *x, int n) {
  uint8_t m = 0;
  for (int i = 0; i < n; i++) m = max8(m, x[i]);
  return m;
}

uint16_t maxv16(uint16_t *x, int n) {
  uint16_t m = 0;
  for (int i = 0; i < n; i++) m = max16(m, x[i]);
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

//------------------------------------------------------------------------------
// RNG / error-check helpers
//------------------------------------------------------------------------------

bool check_balanced_counts(int *counts, int n) {
  double q = TEST_P_VALUE;
  double mean, std, z, r, N = 0;
  for (int i = 0; i < n; i++) N += counts[i];
  mean = (double)N/n;
  std = sqrt((double)N*(n - 1)/(n*n));
  r = q/n/2;
  z = probit(1 - r);
  printD("counts allowed deviation", std*z);
  printIVS("counts", counts, n);
  for (int i = 0; i < n; i++)
    if (fabs(counts[i] - mean) > std*z) return false;
  return true;
}

bool check_balanced_bits(int *ones, int N, int B) {
  double q = TEST_P_VALUE;
  double r, mean, std, z;
  mean = (double)N/2;
  std = sqrt((double)N/4);
  r = q/B/2;
  z = probit(1 - r);
  printD("bit allowed deviation", std*z);
  printIVS("bit-ones", ones, B);
  for (int b = 0; b < B; b++)
    if (fabs(ones[b] - mean) > std*z) return false;
  return true;
}

void check_rng_clean(randompack_rng *rng) {
  xCheck(rng);
  char *err = randompack_last_error(rng);
  xCheck(!err || !err[0]);
}

void check_success(bool ok, randompack_rng *rng) {
  xCheck(ok);
  char *err = randompack_last_error(rng);
  xCheck(!err || !err[0]);
}

void check_failure(bool ok, randompack_rng *rng) {
  xCheck(!ok);
  char *err = randompack_last_error(rng);
  xCheck(err && err[0]);
}

randompack_rng *create_seeded_rng(const char *engine, int seed) {
  randompack_rng *rng = randompack_create(engine);
  if (rng) randompack_seed(seed, 0, 0, rng);
  return rng;
}
