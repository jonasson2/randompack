#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include "randompack.h"
#include "randompack_config.h"
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

bool equal_int(int *a, int *b, int n) {
  for (int i = 0; i < n; i++) if (a[i] != b[i]) return false;
  return true;
}

bool equal_uint32(uint32_t *a, uint32_t *b, int n) {
  for (int i = 0; i < n; i++) if (a[i] != b[i]) return false;
  return true;
}

bool equal_uint64(uint64_t *a, uint64_t *b, int n) {
  for (int i = 0; i < n; i++) if (a[i] != b[i]) return false;
  return true;
}

bool everywhere_different(uint64_t *a, uint64_t *b, int n) {
  for (int i = 0; i < n; i++) if (a[i] == b[i]) return false;
  return true;
}

uint32_t imax32(uint32_t a, uint32_t b) {
  return a > b ? a : b;
}

uint64_t imax64(uint64_t a, uint64_t b) {
  return a > b ? a : b;
}

int imaxv(int *x, int n) {
  int m = INT_MIN;
  for (int i = 0; i < n; i++) m = imax(m, x[i]);
  return m;
}

uint32_t imaxv32(uint32_t *x, int n) {
  uint32_t m = 0;
  for (int i = 0; i < n; i++) m = imax32(m, x[i]);
  return m;
}

uint64_t imaxv64(uint64_t *x, int n) {
  uint64_t m = 0;
  for (int i = 0; i < n; i++) m = imax64(m, x[i]);
  return m;
}

int iminv(int *x, int n) {
  int m = INT_MAX;
  for (int i = 0; i < n; i++) m = imin(m, x[i]);
  return m;
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
