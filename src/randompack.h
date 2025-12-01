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

typedef struct rand_rng randompack_rng;

randompack_rng *randompack_create( // Create RNG with given type and seed, NULL on error
  const char *type,  // in   Park-Miller/PM, Xorshift128+/Xorshift/X+, R/R-default
  long long seed     // in   0 to randomize, >0 to seed, <0 for thread randomize
);

void randompack_free( // Free an RNG created with randompack_create
  randompack_rng *rng   // in   Random number generator
);

unsigned int randompack_getPMseed( // Return the current Park-Miller seed, 0 on error
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
// NOTE 5: The seed type long long is 64 bits on almost all architectures in 2025. Only
//         the most significant 32 bits are used for Park-Miller seeding. 
// NOTE 6: To use the thread-randomize feature set the seed to -thread_id, to set the
//         random state to a mix of the thread-id and system-entropy.

#endif /* RANDOMPACK_H */
