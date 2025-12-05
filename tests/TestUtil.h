// Various utilities for the test functions
#include "randompack_config.h"

char *engines[] = {"xorshift128+", "xoshiro256**", "xoshiro256++", "chacha20"
#ifdef HAVE128
  , "pcg64"
#endif
};
char *abbrev[] = {"x128+",         "x256**",       "x256++",       "chacha20"
#ifdef HAVE128
  , "pcg"
#endif
};

// Check that int vectors are equal
static inline bool equal_int(int *a, int *b, int n) {
  for (int i=0; i<n; i++) if (a[i] != b[i]) return false;
  return true;
}

// Check that uint32 vectors are equal
static inline bool equal_uint32(uint32_t *a, uint32_t *b, int n) {
  for (int i=0; i<n; i++) if (a[i] != b[i]) return false;
  return true;
}

// Check that uint64 vectors are equal
static inline bool equal_uint64(uint64_t *a, uint64_t *b, int n) {
  for (int i=0; i<n; i++) if (a[i] != b[i]) return false;
  return true;
}

// Check that all entries in a and b differ
static inline bool everywhere_different(uint64_t *a, uint64_t *b, int n) {
  for (int i=0; i<n; i++) if (a[i] == b[i]) return false;
  return true;
}

// Check that counts is consistent with counts in bins with equal probability
bool check_balanced_counts(int *counts, int n);
