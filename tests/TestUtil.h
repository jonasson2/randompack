// Various utilities for the test functions
#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include "randompack_config.h"

static char *engines[] = {"xorshift128+", "xoshiro256**", "xoshiro256++", "chacha20"
#ifdef HAVE128
  , "pcg64"
#endif
};
static char *abbrev[] = {"x128+",         "x256**",       "x256++",       "chacha20"
#ifdef HAVE128
  , "pcg"
#endif
};

enum {
  N_BAL_CNTS = 500000,
  N_BAL_BITS = 40000
};

bool equal_int(int *a, int *b, int n);                      // a = b?
bool equal_uint32(uint32_t *a, uint32_t *b, int n);         // a = b?
bool equal_uint64(uint64_t *a, uint64_t *b, int n);         // a = b?
bool everywhere_different(uint64_t *a, uint64_t *b, int n); // ai ≠ bi for all i
int iminv(int *x, int n);
int imaxv(int *x, int n);
uint32_t imaxv32(uint32_t *x, int n);
uint64_t imaxv64(uint64_t *x, int n);
int imaxv(int *x, int n);

bool check_balanced_counts(int *counts, int n);    // All counts ≈ equal
bool check_balanced_bits(int *ones, int N, int B); // Is each bit is set with ~50% prob?
void check_rng_clean(randompack_rng *rng);         // Is rng ≠ 0 and last_error empty?
void check_success(bool ok, randompack_rng *rng);  // Success and last_error empty?
void check_failure(bool ok, randompack_rng *rng);  // Not ok and last_error ≠ ""
uint32_t imax32(uint32_t a, uint32_t b);
uint64_t imax64(uint64_t a, uint64_t b);
uint32_t imaxv32(uint32_t *x, int n);
uint64_t imaxv64(uint64_t *x, int n);
int imaxv(int *x, int n);

#endif
