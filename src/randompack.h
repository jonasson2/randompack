// randompack — random number generation utilities for VARMASIM
//
// This header provides user-facing functions for generating random values
// drawn from uniform, normal, and multivariate normal distributions, along
// with convenience wrappers for creating, seeding, and randomizing RNGs.
//
// DEPENDENCIES:
//   It builds on the low-level RNG framework defined in random.h, which is
//   included below so that users need not include it themselves (but can
//   for extra features, e.g. complete control over rng state).
//
// INTEGRATION WITH R:
//   When compiled with -DUSING_R, the default RNG redirects calls to R's
//   built-in random number generator for reproducibility inside R packages.
//   Otherwise, the standalone implementation uses Xorshift128+ as the
//   DEFAULT_RNG and Park–Miller for cross-platform comparison.
//
// NOTES:
//   Implementation details and additional references are in RandomNumbers.c.

#ifndef RANDOMPACK_H
#define RANDOMPACK_H

#include <stdbool.h>
#include <stdint.h>

typedef struct randompack_rng randompack_rng;
typedef struct { uint64_t v[4]; } randompack_counter, randompack_3fry_key;
typedef struct { uint64_t v[2]; } randompack_philox_key;

randompack_rng *randompack_create( // Create RNG with given type and seed, NULL on error
  const char *type,  // in   Park-Miller/PM, Xorshift128+/Xorshift/X+, R/R-default
  int seed           // in   0 to randomize, >0 to seed, <0 for thread randomize
);

void randompack_free( // Free an RNG created with randompack_create
  randompack_rng *rng   // in   Random number generator
);

bool randompack_set_norm_method( // Set algorithm used for random normals
  char *method,         // in   "polar" or "default" (for ziggurat)
  randompack_rng *rng   // in   Random number generator
);

bool randompack_u01( // Generate uniform random numbers in [0,1), false on error
  double x[],           // out  n-vector: uniform random numbers in [0,1)
  int n,                // in   Number of variates
  randompack_rng *rng   // in   Random number generator
);

bool randompack_int( // Generate uniform integers in [m, n], false on error
  int x[],              // out  len-vector of integers
  int len,              // in   Number of integers requested
  int m,                // in   Inclusive minimum
  int n,                // in   Inclusive maximum
  randompack_rng *rng   // in   Random number generator
);

bool randompack_perm( // Generate a random permutation of 0..n-1, false on error
  int x[],              // out  n-vector containing the permutation
  int n,                // in   Permutation size
  randompack_rng *rng   // in   Random number generator
);

bool randompack_sample( // Sample without replacement from 0..n-1, false on error
  int x[],              // out  k-vector of sampled indices
  int n,                // in   Population size
  int k,                // in   Sample size (0 <= k <= n)
  randompack_rng *rng   // in   Random number generator
);

bool randompack_norm( // Generate standard normal random numbers N(0,1), false on error
  double x[],           // out  n-vector: standard normal random numbers
  int n,                // in   Number of variates
  randompack_rng *rng   // in   Random number generator
);

bool randompack_mvn( // Generate multivariate normal randoms N(mu,Sig), false on error
  char *transp,         // in     "N" to get n×d X, "T" to get d×n X
  double mu[],          // in     d-vector: mean (NULL → zero-mean)
  double Sig[],         // in     d×d covariance matrix (NULL → use L as-is)
  int d,                // in     Dimension of each vector
  int n,                // in     Number of replicates
  double X[],           // out    n×d or d×n matrix of generated vectors
  int ldx,              // in     Leading dimension of X
  double L[],           // in/out d×d lower Cholesky factor of Sig (or NULL)
  randompack_rng *rng   // in     Random number generator
);

bool randompack_uint32( // Generate uint32 in [0, bound), false on error
  uint32_t x[],          // out  len-vector of integers
  int len,               // in   Number requested
  uint32_t bound,        // in   Exclusive upper bound, or 0 for unbounded
  randompack_rng *rng    // in   Random number generator
);

bool randompack_uint64( // Generate uint64 in [0, bound), false on error
  uint64_t x[],          // out  len-vector of integers
  int len,               // in   Number requested
  uint64_t bound,        // in   Exclusive upper bound, or 0 for unbounded
  randompack_rng *rng    // in   Random number generator
);

bool randompack_uint64_3fry(
  uint64_t x[],          // out  len-vector of integers (unbounded)
  int len,               // in   Number requested
  randompack_counter ctr,// in   Counter state
  randompack_3fry_key key// in   Threefry4x64 key
);

bool randompack_get_state( // Serialize RNG state to an opaque buffer
  int *len,                 // in/out 0→query size; otherwise buffer length
  uint8_t *buf,             // out    state buffer (may be NULL if *len==0)
  randompack_rng *rng       // in     RNG whose state to serialize
);

bool randompack_set_state( // Restore RNG state from an opaque buffer
  int len,                  // in  buffer length
  const uint8_t *buf,       // in  serialized state
  randompack_rng *rng       // in  target RNG (must be allocated)
);

const char *randompack_last_error( // Get last error string, or 0 if none
  const randompack_rng *rng        // in  RNG
);

// NOTE 1: Sig, X and L are stored columnwise in Fortran fashion.
// NOTE 2: There are situations when Sig is indefinite but close to being positive
//         definite, for example due to rounding errors. To remedy this a pivoted
//         Cholesky factorization of Sig is employed, which may return an L matrix
//         which is not lower and has some trailing columns 0, but does satisfy
//         L·L' = Sig, and can thus be used to generate vectors with the right
//         distribution.
// NOTE 3: In case the calling program needs the Cholesky factor of Sig this may
//         be returned in L by letting it be a d × d array instead of 0 in the
//         call.
// NOTE 4: When randompack_mvn is to be called multiple times for the same
//         covariance it is possible to save execution time by reusing the
//         Cholesky factorization of Sig on all calls but the first. Specify Sig
//         and return L on the first call, and let Sig be null and specify L on
//         subsequent calls. If both Sig and L are null, the function exits with
//         ok = 0.
// NOTE 5: The seed type int is 32 bits; only the lower 31 bits are used for Park-Miller
//         seeding.
// NOTE 6: To use the thread-randomize feature set the seed to -thread_id to mix the
//         thread-id with system entropy.

#endif /* RANDOMPACK_H */
