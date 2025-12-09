// Various utilities for the test functions
#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include "randompack.h"
#include "randompack_config.h"

//------------------------------------------------------------------------------
// Engine name tables used by tests
//------------------------------------------------------------------------------

static char *engines[] = {
  "xorshift128+",
  "xoshiro256**",
  "xoshiro256++",
  "chacha20"
#ifdef HAVE128
  , "pcg64"
#endif
};

static char *abbrev[] = {
  "x128+",
  "x256**",
  "x256++",
  "chacha20"
#ifdef HAVE128
  , "pcg"
#endif
};

//------------------------------------------------------------------------------
// Global test sizes / sample counts
//------------------------------------------------------------------------------

enum {
  N_BAL_CNTS = 500000,
  N_BAL_BITS = 40000,
  N_statistics = 100000
};
static const double TEST_P_VALUE = 1e-3;

//------------------------------------------------------------------------------
// Vector equality / difference helpers
//------------------------------------------------------------------------------

bool equal_vec(int *a, int *b, int n);                      // a = b?
bool equal_vecd(double *a, double *b, int n);               // a = b?
bool equal_vec32(uint32_t *a, uint32_t *b, int n);          // a = b?
bool equal_vec64(uint64_t *a, uint64_t *b, int n);          // a = b?
bool everywhere_different(uint64_t *a, uint64_t *b, int n); // ai ≠ bi for all i

//------------------------------------------------------------------------------
// Approximate equality
//------------------------------------------------------------------------------

int almostSame(double a, double b);
int almostEqual(double a[], double b[], int n);
int almostAllSame(double a[], int n);
int almostZero(double a[], int n);

//------------------------------------------------------------------------------
// Simple statistics
//------------------------------------------------------------------------------

double mean(double *x, int n);
double var(double *x, int n, double mu);
double skewness(double *x, int n, double xbar, double s2);
double kurtosis(double *x, int n, double xbar, double s2);
void cov(char *transp, int m, int n, double X[], double C[]);
bool check_meanvar(double *x, int n, double mu, double s2, double stdmu, double stds2);
bool check_skew(double *x, int n, double xbar, double s2, double skew, double skew_std);
bool check_kurt(double *x, int n, double xbar, double s2, double kurt, double kurt_std);
double probit(double p);

//------------------------------------------------------------------------------
// Min/max helpers for scalars and vectors
//------------------------------------------------------------------------------

uint32_t max32(uint32_t a, uint32_t b);
uint64_t max64(uint64_t a, uint64_t b);

int maxv(int *x, int n);
uint32_t maxv32(uint32_t *x, int n);
uint64_t maxv64(uint64_t *x, int n);
double maxvd(double *x, int n);

int minv(int *x, int n);
double minvd(double *x, int n);

//------------------------------------------------------------------------------
// RNG / error-check helpers
//------------------------------------------------------------------------------

bool check_balanced_counts(int *counts, int n);    // All counts ≈ equal
bool check_balanced_bits(int *ones, int N, int B); // Is each bit set with ~50% prob?

void check_rng_clean(randompack_rng *rng);         // rng ≠ 0 and last_error empty?
void check_success(bool ok, randompack_rng *rng);  // ok and last_error empty?
void check_failure(bool ok, randompack_rng *rng);  // !ok and last_error ≠ ""

randompack_rng *create_seeded_rng(const char *engine, int seed);

#endif
