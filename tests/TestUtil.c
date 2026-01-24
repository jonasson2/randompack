#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "TestUtil.h"
#include "randompack.h"
#include "randompack_config.h"
#include "MatrixTestUtil.h"
#include "xCheck.h"
#include "printX.h"

// Define test sample sizes
// ––––––––––––––––––––––––
int N_BAL_CNTS = N_BAL_CNTS_DEFAULT;
int N_BAL_BITS = N_BAL_BITS_DEFAULT;
int N_STAT_FAST = N_STAT_FAST_DEFAULT;
int N_STAT_SLOW = N_STAT_SLOW_DEFAULT;
char **get_engines(int *n) {
  int n0 = 0;
  int emax = 0;
  int dmax = 0;
  char *names = 0;
  char *descriptions = 0;
  char **engines = 0;
  if (!n) return 0;
  *n = 0;
  bool ok = randompack_engines(0, 0, &n0, &emax, &dmax);
  ASSERT(ok);
  ASSERT(n0 > 0 && emax > 0 && dmax > 0);
  TEST_ALLOC(names, n0*emax);
  TEST_ALLOC(descriptions, n0*dmax);
  ok = randompack_engines(names, descriptions, &n0, &emax, &dmax);
  ASSERT(ok);
  int m = 0;
  for (int i = 0; i < n0; i++) {
    char *name = names + i*emax;
    if (strcmp(name, "system") != 0) m++;
  }
  ASSERT(m > 0);
  TEST_ALLOC(engines, m);
  int k = 0;
  for (int i = 0; i < n0; i++) {
    char *name = names + i*emax;
    if (strcmp(name, "system") == 0) continue;
    int len = (int)strlen(name);
    char *copy;
    TEST_ALLOC(copy, len + 1);
    memcpy(copy, name, len + 1);
    engines[k++] = copy;
  }
  FREE(descriptions);
  FREE(names);
  *n = m;
  return engines;
}
void free_engines(char **engines, int n) {
  if (!engines) return;
  for (int i = 0; i < n; i++) FREE(engines[i]);
  FREE(engines);
}

// Vector equality / difference helpers
// ––––––––––––––––––––––––––––––––––––
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

#include <math.h>

static int cmp_double(const void *a, const void *b) {
  double da = *(const double *)a;
  double db = *(const double *)b;
  int a_nan = isnan(da);
  int b_nan = isnan(db);
  if (a_nan || b_nan) return a_nan - b_nan; // non-NaN first, NaN last
  if (da < db) return -1;
  if (da > db) return 1;
  return 0;
}

void print_lowhigh(char *name, double *x, int n, int ndec) {
  (void) name; (void) x; (void) n; (void) ndec; // prevent unused warnings
  if (printIsOff()) return;
  if (n <= 0) return;
  int nlow = n < 8 ? n : 8; (void) nlow;
  int nhigh = n < 8 ? n : 8; (void) nhigh;
  double *tmp;
  TEST_ALLOC(tmp, n);
  for (int i = 0; i < n; i++) tmp[i] = x[i];
  qsort(tmp, n, sizeof(tmp[0]), cmp_double);
  int old_ndec = printGetNdec(); (void) old_ndec;
  printSetNdec(ndec);
  printMsg(name);
  printV("  low", tmp, nlow);
  printV("  high", tmp + n - nhigh, nhigh);
  printSetNdec(old_ndec);
  FREE(tmp);
}

bool everywhere_different(uint64_t *a, uint64_t *b, int n) {
  for (int i = 0; i < n; i++) if (a[i] == b[i]) return false;
  return true;
}

// Approximate equality
// ––––––––––––––––––––
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

// Simple statistics
// –––––––––––––––––
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
  TEST_ALLOC(mu, n);
  TEST_ALLOC(Xm, m*n);
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

// Inverse normal CDF. Translation of PPND16 in AS241 (Wichura, 1988), public domain
// –––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
double probit(double p) { // Return NAN if p < 0 or p > 1
  if (p < 0 || p > 1) return NAN;
  if (p == 0) return -DBL_MAX;  // New in the translation
  if (p == 1) return  DBL_MAX;  //
  p = fmax(p, nextafter(0.0, 1.0)); //
  p = fmin(p, nextafter(1.0, 0.0)); //
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

double normcdf(double x) {
  return 0.5*erfc(-x/sqrt(2.0));
}

double normccdf(double x) {
  return 0.5*erfc(x/sqrt(2.0));
}

double chi2_cdf(double x, int nu) {
  if (x <= 0.0) return 0.0;
  return igam(0.5*nu, 0.5*x);
}

double chi2_ccdf(double x, int nu) {
  if (x <= 0.0) return 1.0;
  return igamc(0.5*nu, 0.5*x);
}

double gamma_cdf(double x, double shape, double scale) {
  if (x <= 0.0) return 0.0;
  return igam(shape, x/scale);
}

double gamma_cdfc(double x, double shape, double scale) {
  if (x <= 0.0) return 1.0;
  return igamc(shape, x/scale);
}

// Min/max helpers for scalars and vectors
// –––––––––––––––––––––––––––––––––––––––
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

// RNG / error-check helpers
// –––––––––––––––––––––––––
bool check_balanced_counts(int *counts, int n) {
  // Pearson chi-square goodness-of-fit for Multinomial(N; 1/n,...,1/n)
  if (n <= 1) return true;
  double N = 0.0;
  for (int i = 0; i < n; i++) N += counts[i];
  if (N <= 0.0) return true;
  double mean = N/n;
  double x2 = 0.0;
  for (int i = 0; i < n; i++) {
    double d = counts[i] - mean;
    x2 += d*d/mean;
  }
  double p = chi2_ccdf(x2, n - 1); // upper tail
  printD("balanced counts p-value", p);
  printIVS("counts", counts, n);
  return p >= TEST_P_VALUE;
}

bool check_balanced_bits(int *ones, int N, int B) {
  // Pearson chi-square goodness-of-fit for B Bernoulli(0.5) variables
  if (B <= 1 || N <= 0) return true;
  double mean = N/2.0;
  double x2 = 0.0;
  for (int b = 0; b < B; b++) {
    double d = ones[b] - mean;
    x2 += d*d/mean;
  }
  double p = chi2_ccdf(x2, B - 1); // upper tail
  printD("balanced bits p-value", p);
  printIVS("bit-ones", ones, B);
  return p >= TEST_P_VALUE;
}

void check_rng_clean(randompack_rng *rng) {
  ASSERT(rng);
  char *err = randompack_last_error(rng);
  ASSERT(!err || !err[0]);
}

void check_success(bool ok, randompack_rng *rng) {
  ASSERT(ok);
  char *err = randompack_last_error(rng);
  ASSERT(!err || !err[0]);
}

void check_failure(bool ok, randompack_rng *rng) {
  ASSERT(!ok);
  char *err = randompack_last_error(rng);
  ASSERT(err && err[0]);
}

randompack_rng *create_seeded_rng(const char *engine, int seed) {
  randompack_rng *rng = randompack_create(engine);
  if (rng) randompack_seed(seed, 0, 0, rng);
  return rng;
}

bool check_meanvar(double *x, int n) {
  // Mean and variance test for a standard normal x
  if (n <= 1) return true;
  double mu = 0, sigma = 1, xbar, s2, xbarstd, zstat, p_mean, t, p_var;
  xbar = mean(x, n);
  s2 = var(x, n, xbar);
  printD("xbar", xbar);
  xbarstd = sigma/sqrt(n);
  zstat = fabs(xbar - mu)/xbarstd;
  p_mean = 2.0*normccdf(zstat); // Mean test: (xbar - mu)/(sigma/sqrt(n)) ~ N(0,1)
  t = n*s2;                     // Variance test: n*s2 ~ chi^2_{n-1}
  p_var = 2.0*fmin(chi2_cdf(t, n - 1), chi2_ccdf(t, n - 1));
  return p_mean >= TEST_P_VALUE && p_var >= TEST_P_VALUE;
}

bool check_skewkurt(double *x, int n, double skew, double kurt) {
  // Skewness and kurtosis tests for a standard normal x
  if (n <= 3) return true;
  double xbar, s2, skew_hat, kurt_hat, skew_std, kurt_std, z_skew, z_kurt, p_skew, p_kurt;
  xbar = mean(x, n);
  s2 = var(x, n, xbar);
  skew_hat = skewness(x, n, xbar, s2);
  kurt_hat = kurtosis(x, n, xbar, s2);
  skew_std = sqrt(6.0/n);
  kurt_std = sqrt(24.0/n);
  z_skew = fabs(skew_hat - skew)/skew_std;
  z_kurt = fabs(kurt_hat - kurt)/kurt_std;
  p_skew = 2.0*normccdf(z_skew);
  p_kurt = 2.0*normccdf(z_kurt);
  return p_skew >= TEST_P_VALUE && p_kurt >= TEST_P_VALUE;
}

bool check_u01_meanvar(double *x, int n) {
  // Mean/variance test for U01 via probit -> N(0,1)
  if (n <= 1) return true;
  double *z;
  TEST_ALLOC(z, n);
  for (int i = 0; i < n; i++) z[i] = probit(x[i]);
  bool ok = check_meanvar(z, n);
  FREE(z);
  return ok;
}

bool check_u01_skewkurt(double *x, int n) {
  // Skewness/kurtosis test for U01 via probit -> N(0,1) (normal approximation)
  if (n <= 3) return true;
  double *z;
  TEST_ALLOC(z, n);
  for (int i = 0; i < n; i++) z[i] = probit(x[i]);
  bool ok = check_skewkurt(z, n, 0.0, 3.0);
  FREE(z);
  return ok;
}

bool check_u01_minmax(double *x, int n) {
  if (n <= 0) return true;
  const double q = TEST_P_VALUE;
  double lo = q/2/n;      // ≈ Betainv(q/2, 1, n)
  double hi = log(2/q)/n; // ≈ Betainv(1-q/2, n, 1)
  double xmin = minvd(x, n);
  double xmax = maxvd(x, n);
  if (xmin < lo || xmin > hi) return false;
  if (1 - xmax < lo || 1 - xmax > hi) return false;
  return true;
}

// PIT test helpers
// ––––––––––––––––
static bool check_u01_endpoints(int k0, int k1, size_t n, double p) {
  // k0 = count of u=0, k1 = count of u=1, p = ziggurat probability of u = 0
  double lambda = n*p;
  double pv0 = (k0 == 0) ? 1.0 : igam((double)k0, lambda);
  double pv1 = (k1 == 0) ? 1.0 : igam((double)k1, lambda);
  return (pv0 >= TEST_P_VALUE && pv1 >= TEST_P_VALUE);
}

static void check_u01_distribution_df(double *u, int n, char *dist, char *engine, char *
												  precision) {
  int k0 = 0;
  int k1 = 0;
  int nnz = 0;
  for (int i = 0; i < n; i++) {
    double x = u[i];
    if (x == 0.0) k0++;
    else if (x == 1.0) k1++;
    else u[nnz++] = x;
	 // u[nnz++] = x;
  }
  double p; // ziggurat resolution
  if (!strcmp(precision, "float"))
	 p = ldexp(1.0, -23); // 2^(-23)
  else
	 p = ldexp(1.0, -52); // 2^(-52)
  xCheckMsg2(check_u01_endpoints(k0, k1, n, p), engine, dist);
  int nbins = min(500, max(20, sqrt(nnz)));
  ASSERT(nnz/nbins >= 20);
  int *counts;
  TEST_ALLOC(counts, nbins);
  for (int b = 0; b < nbins; b++) counts[b] = 0;
  xCheckMsg2(check_u01_meanvar(u, nnz), engine, dist);
  xCheckMsg2(check_u01_skewkurt(u, nnz), engine, dist);
  xCheckMsg2(check_u01_minmax(u, nnz), engine, dist);
  for (int i = 0; i < nnz; i++) {
    int b = min(nbins - 1, nbins*u[i]);
    counts[b]++;
  }
  xCheckMsg2(check_balanced_counts(counts, nbins), engine, dist);
  FREE(counts);
}

void check_u01_distribution(double *u, int n, char *dist, char *engine) {
  check_u01_distribution_df(u, n, dist, engine, "double");
}

void check_u01_distributionf(float *u, int n, char *dist, char *engine) {
  double *y;
  TEST_ALLOC(y, n);
  for (int i = 0; i < n; i++) y[i] = u[i];
  check_u01_distribution_df(y, n, dist, engine, "float");
  FREE(y);
}
