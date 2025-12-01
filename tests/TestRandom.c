// TestRandom.c — unit tests for RandomNumbers using ExtraUtil helpers

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include "xCheck.h"

#include "randompack.h"
#include "VarmaUtilities.h"
#include "BlasGateway.h"
#include "ExtraUtil.h"    // for mean, var, relabsdiff, almostSame, almostEqual
#include "allocate.h"
#include "printX.h"

static int min_intv(const int *x, int n) {
  int m = x[0];
  for (int i = 1; i < n; i++) {
    if (x[i] < m) m = x[i];
  }
  return m;
}

static int max_intv(const int *x, int n) {
  int m = x[0];
  for (int i = 1; i < n; i++) {
    if (x[i] > m) m = x[i];
  }
  return m;
}

static bool equal_intv_offset(const int *a, const int *b, int n, int offset) {
  for (int i = 0; i < n; i++) {
    if (b[i] != a[i] + offset) return false;
  }
  return true;
}

static bool is_perm_0_to_n_minus1(const int *x, int n) {
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

// Test uniform generator
static void test_uniform_basic(void) {
  const int N = 1e6;
  double meantol = 7*1/sqrt(12*N);  // 7 sigma where sigma = 1/sqrt(12·N)
  double vartol = 7*1/sqrt(180*N);  // 7 std of variance of U(0,1)
  double exactmu = 0.5, exactvar = 1.0/12.0;
  double *x = NULL;
  allocate(x, N);

  randompack_rng *rng = randompack_create("Xorshift", 0);
  randompack_u01(x, N, rng);

  // Check range
  double xmin = x[0], xmax = x[0];
  for (int i = 1; i < N; i++) {
    if (x[i] < xmin) xmin = x[i];
    if (x[i] > xmax) xmax = x[i];
  }
  xCheck(xmin >= 0.0 && xmax < 1.0);
  double mu = mean(x, N);
  double va = var(x, N, mu);
  xCheck(fabs(mu - exactmu) < meantol);
  xCheck(fabs(va - exactvar) < vartol);

  randompack_free(rng);
  freem(x);
}

// Test normal generator
static void test_normal_basic(void) {
  const int N = 1e6;
  double meantol = 7*1/sqrt(N);
  double vartol = 7*sqrt(2.0/(N-1)); // 7 std of variance of U(0,1)
  double exactmu = 0.0, exactvar = 1.0;
  double *x = NULL;
  allocate(x, N);

  randompack_rng *rng = randompack_create("Xorshift", 0);
  randompack_norm(x, N, rng);
  double mu = mean(x, N);
  double va = var(x, N, mu);
  printD("va", va);
  printD("exactvar", exactvar);
  printD("vartol", vartol);
  xCheck(fabs(mu - exactmu) < meantol);
  xCheck(fabs(va - exactvar) < vartol);

  randompack_free(rng);
  freem(x);
}

// Test 

// Test determinism under same seed and RNG type
static void test_determinism_default_seed(void) {
  const int N = 1000;
  double *a = NULL, *b = NULL;
  allocate(a, N);
  allocate(b, N);

  randompack_rng *r1 = randompack_create("Xorshift", 123);
  randompack_rng *r2 = randompack_create("Xorshift", 123);

  randompack_norm(a, N, r1);
  randompack_norm(b, N, r2);

  xCheck(almostEqual(a, b, N));

  randompack_free(r1);
  randompack_free(r2);
  freem(a);
  freem(b);
}

// Test switching RNG type affects output
static void test_pm_vs_default_selection(void) {
  const int N = 256;
  double *a = NULL, *b = NULL;
  allocate(a, N);
  allocate(b, N);

  randompack_rng *r1 = randompack_create("Xorshift", 123);
  randompack_u01(a, N, r1);

  randompack_rng *r2 = randompack_create("PM", 123);
  randompack_u01(b, N, r2);

  xCheck(!almostEqual(a, b, N));

  randompack_free(r1);
  randompack_free(r2);
  freem(a);
  freem(b);
}

// Test randomize changes stream
static void test_randomize_changes_stream(void) {
  const int N = 128;
  double *a, *b, *c;
  allocate(a, N);
  allocate(b, N);
  allocate(c, N);

  randompack_rng *r1 = randompack_create("Xorshift", 42);
  randompack_u01(a, N, r1);

  randompack_rng *r2 = randompack_create("Xorshift", 0);
  randompack_u01(b, N, r2);

  randompack_rng *r3 = randompack_create("Xorshift", -42);
  randompack_u01(c, N, r3);

  xCheck(!almostEqual(a, b, N));
  xCheck(!almostEqual(a, c, N));
  xCheck(!almostEqual(b, c, N));

  randompack_free(r1);
  randompack_free(r2);
  randompack_free(r3);
  freem(a);
  freem(b);
  freem(c);
}

static void test_pm_seed_updates(void) {
  const int N = 4;
  double x[N];
  randompack_rng *rng = randompack_create("PM", 123);
  unsigned int s1 = randompack_getPMseed(rng);
  unsigned int s2 = randompack_getPMseed(rng);
  xCheck(s1 == s2);
  randompack_u01(x, N, rng);
  unsigned int s3 = randompack_getPMseed(rng);
  xCheck(s3 != s2);
  randompack_free(rng);
}

static void test_int_api(void) {
  const int N = 128;
  int a[N], b[N];
  randompack_rng *r1 = randompack_create("Xorshift", 99);
  randompack_rng *r2 = randompack_create("Xorshift", 99);
  randompack_int(a, N, -3, 8, r1);
  randompack_int(b, N, 0, 11, r2);
  xCheck(min_intv(a, N) >= -3 && max_intv(a, N) <= 8);
  xCheck(min_intv(b, N) >= 0 && max_intv(b, N) <= 11);
  xCheck(equal_intv_offset(a, b, N, 3));
  randompack_free(r1);
  randompack_free(r2);
}

static void test_perm_api(void) {
  const int N = 32;
  int perm[N];
  randompack_rng *rng = randompack_create("Xorshift", 77);
  randompack_perm(perm, N, rng);
  xCheck(min_intv(perm, N) >= 0 && max_intv(perm, N) < N);
  xCheck(is_perm_0_to_n_minus1(perm, N));
  randompack_free(rng);
}

static void test_sample_api(void) {
  const int N = 50;
  const int K = 10;
  int sample1[K], sample2[K];
  int used[N];
  randompack_rng *r1 = randompack_create("Xorshift", 11);
  randompack_rng *r2 = randompack_create("Xorshift", 11);
  randompack_sample(sample1, N, K, r1);
  randompack_sample(sample2, N, K, r2);
  for (int i = 0; i < N; i++) used[i] = 0;
  for (int i = 0; i < K; i++) {
    int v = sample1[i];
    xCheck(v >= 0 && v < N);
    xCheck(!used[v]);
    used[v] = 1;
    xCheck(sample1[i] == sample2[i]);
  }
  randompack_rng *r3 = randompack_create("Xorshift", 5);
  randompack_sample(0, N, 0, r3); // ensure k=0 is a no-op
  randompack_free(r1);
  randompack_free(r2);
  randompack_free(r3);
}

// Test type name aliases produce identical output
static void test_type_name_aliases(void) {
  const int N = 100;
  double *a = NULL, *b = NULL, *c = NULL, *d = NULL, *e = NULL;
  allocate(a, N);
  allocate(b, N);
  allocate(c, N);
  allocate(d, N);
  allocate(e, N);

  randompack_rng *r1 = randompack_create("Xorshift128+", 123);
  randompack_u01(a, N, r1);

  randompack_rng *r2 = randompack_create("Xorshift", 123);
  randompack_u01(b, N, r2);

  randompack_rng *r3 = randompack_create("X+", 123);
  randompack_u01(c, N, r3);

  xCheck(almostEqual(a, b, N));
  xCheck(almostEqual(a, c, N));

  randompack_free(r1);
  randompack_free(r2);
  randompack_free(r3);

  randompack_rng *r4 = randompack_create("Park-Miller", 456);
  randompack_u01(d, N, r4);

  randompack_rng *r5 = randompack_create("PM", 456);
  randompack_u01(e, N, r5);

  xCheck(almostEqual(d, e, N));

  randompack_free(r4);
  randompack_free(r5);

  randompack_rng *r6 = randompack_create("R", 789);
  randompack_u01(a, N, r6);

  randompack_rng *r7 = randompack_create("R-default", 789);
  randompack_u01(b, N, r7);

  xCheck(almostEqual(a, b, N));

  randompack_free(r6);
  randompack_free(r7);

  freem(a);
  freem(b);
  freem(c);
  freem(d);
  freem(e);
}


// static void run_test(const char *name, void (*fn)(void)) {
//   xCheckAddMsg(name);
//   fn();
// }

#define RUN_TEST(x)                             \
 do {                                           \
   xCheckAddMsg(#x);                            \
   test_##x();                                  \
 } while(0)

void TestRandomNumbers(void) {
  RUN_TEST(uniform_basic);
  RUN_TEST(normal_basic);
  RUN_TEST(determinism_default_seed);
  RUN_TEST(pm_vs_default_selection);
  RUN_TEST(randomize_changes_stream);
   RUN_TEST(pm_seed_updates);
  RUN_TEST(int_api);
  RUN_TEST(perm_api);
  RUN_TEST(sample_api);
  RUN_TEST(type_name_aliases);
}
