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
#include <stddef.h>

typedef struct randompack_rng randompack_rng;
typedef struct { uint64_t v[4]; } randompack_counter, randompack_3fry_key;
typedef struct { uint64_t v[2]; } randompack_philox_key;

randompack_rng *randompack_create( // Create randomized RNG of given engine type, error→0
  const char *engine    // in      Engine name (Park-Miller, PCG, Xoshiro256++,...)
);

bool randompack_seed( // Create RNG with given type and seed, false on error
  int seed,             // in      Any integer seed; expanded with a hash to fill state
  uint32_t *spawn_key,  // in      Optional spawn key array (may be 0 if n_key==0)
  int n_key,            // in      Number of spawn_key entries
  randompack_rng *rng   // in/out  Random number generator
);

void randompack_free( // Free an RNG created with randompack_create
  randompack_rng *rng   // in      Random number generator
);

bool randompack_int( // Generate uniform integers in [m, n], false on error
  int x[],              // out     len-vector of integers
  size_t len,           // in      Number of integers requested
  int m,                // in      Inclusive minimum
  int n,                // in      Inclusive maximum
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_perm( // Generate a random permutation of 0..n-1, false on error
  int x[],              // out     n-vector containing the permutation
  int n,                // in      Permutation size
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_sample( // Sample without replacement from 0..n-1, false on error
  int x[],              // out     k-vector of sampled indices
  int n,                // in      Population size
  int k,                // in      Sample size (0 <= k <= n)
  randompack_rng *rng   // in/out  Random number generator
);

double randompack_u01_draw( // Draw a single uniform random from [0,1)
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_u01( // Generate uniform random numbers in [0,1), false on error
  double x[],           // out     n-vector: uniform random numbers in [0,1)
  size_t n,             // in      Number of variates
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_unif( // Generate uniform random numbers in [a,b], false on error
  double x[],           // out     n-vector: uniform random numbers in [a,b]
  size_t n,             // in      Number of variates
  double a,             // in      Minimum (a < b)
  double b,             // in      Maximum (a < b)
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_norm( // Generate standard normal random numbers N(0,1), false on error
  double x[],           // out     n-vector: standard normal random numbers
  size_t n,             // in      Number of variates
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_normal( // Generate normal random numbers N(mu,sigma), false on error
  double x[],           // out     n-vector: normal random numbers
  size_t n,             // in      Number of variates
  double mu,            // in      Mean
  double sigma,         // in      Standard deviation (> 0)
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_lognormal( // Generate lognormal random numbers, false on error
  double x[],              // out     n-vector: lognormal random numbers
  size_t n,                // in      Number of variates
  double mu,               // in      Mean of underlying normal
  double sigma,            // in      Std dev of underlying normal (> 0)
  randompack_rng *rng      // in/out  Random number generator
);

bool randompack_gumbel( // Generate Gumbel random numbers, false on error
  double x[],           // out     n-vector: Gumbel random numbers
  size_t n,             // in      Number of variates
  double mu,            // in      Location parameter
  double beta,          // in      Scale parameter (> 0)
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_pareto( // Generate Pareto (Type I) random numbers, false on error
  double x[],           // out     n-vector: Pareto random numbers
  size_t n,             // in      Number of variates
  double xm,            // in      Minimum (scale) parameter (> 0)
  double alpha,         // in      Shape parameter (> 0)
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_exp( // Generate exponential random numbers, false on error
  double x[],           // out     n-vector: exponential random numbers
  size_t n,             // in      Number of variates
  double scale,         // in      Scale parameter (1.0 → standard exponential)
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_gamma( // Generate gamma random numbers, false on error
  double x[],           // out     n-vector: gamma random numbers
  size_t n,             // in      Number of variates
  double shape,         // in      Shape parameter (> 0)
  double scale,         // in      Scale parameter (> 0)
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_chi2( // Generate chi-square random numbers, false on error
  double x[],           // out     n-vector: chi-square random numbers
  size_t n,             // in      Number of variates
  double nu,            // in      Degrees of freedom (> 0)
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_beta( // Generate beta random numbers, false on error
  double x[],           // out     n-vector: beta random numbers
  size_t n,             // in      Number of variates
  double a,             // in      Shape parameter a (> 0)
  double b,             // in      Shape parameter b (> 0)
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_t( // Generate Student t random numbers, false on error
  double x[],           // out     n-vector: Student t random numbers
  size_t n,             // in      Number of variates
  double nu,            // in      Degrees of freedom (> 0)
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_f( // Generate F random numbers, false on error
  double x[],           // out     n-vector: F random numbers
  size_t n,             // in      Number of variates
  double nu1,           // in      Numerator degrees of freedom (> 0)
  double nu2,           // in      Denominator degrees of freedom (> 0)
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_weibull( // Generate Weibull random numbers, false on error
  double x[],           // out     n-vector: Weibull random numbers
  size_t n,             // in      Number of variates
  double shape,         // in      Shape parameter (> 0)
  double scale,         // in      Scale parameter (> 0)
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_mvn( // Generate multivariate normal randoms N(mu,Sig), false on error
  char *transp,         // in      "N" to get n×d X, "T" to get d×n X
  double mu[],          // in      d-vector: mean (NULL → zero-mean)
  double Sig[],         // in      d×d covariance matrix (NULL → use L as-is)
  int d,                // in      Dimension of each vector
  size_t n,             // in      Number of replicates
  double X[],           // out     n×d or d×n matrix of generated vectors
  int ldx,              // in      Leading dimension of X
  double L[],           // in/out  d×d lower Cholesky factor of Sig (or NULL)
  randompack_rng *rng   // in/out  Random number generator
);

char *randompack_last_error( // Get last error string, or 0 if none
  randompack_rng *rng   // in      Random number generator
);

//========================================================================================
// Advanced API: Low-level utilities and engine-specific features
//
// These functions expose additional control over RNG behaviour, distribution kernels, and
// bit-precise integer generation. They are intended for specialised use cases, testing,
// and performance tuning, and are typically not needed in routine applications.
//========================================================================================

bool randompack_uint8( // Generate uint8 in [0, bound), false on error
  uint8_t x[],           // out     len-vector of bytes
  size_t len,            // in      number requested
  uint8_t bound,         // in      exclusive upper bound, or 0 for unbounded
  randompack_rng *rng    // in/out  random number generator
);

bool randompack_uint16( // Generate uint16 in [0, bound), false on error
  uint16_t x[],          // out     len-vector of short integers
  size_t len,            // in      number requested
  uint16_t bound,        // in      exclusive upper bound, or 0 for unbounded
  randompack_rng *rng    // in/out  random number generator
);

bool randompack_uint32( // Generate uint32 in [0, bound), false on error
  uint32_t x[],          // out     len-vector of integers
  size_t len,            // in      number requested
  uint32_t bound,        // in      exclusive upper bound, or 0 for unbounded
  randompack_rng *rng    // in/out  random number generator
);

bool randompack_uint64( // Generate uint64 in [0, bound), false on error
  uint64_t x[],          // out     len-vector of uint64_t integers
  size_t len,            // in      number requested
  uint64_t bound,        // in      exclusive upper bound, or 0 for unbounded
  randompack_rng *rng    // in/out  random number generator
);

bool randompack_uint64_3fry( // Counter based random number generation with "threefry"
  uint64_t x[],             // out  len-vector of integers (unbounded)
  size_t len,               // in   number requested
  randompack_counter ctr,   // in   counter state
  randompack_3fry_key key   // in   threefry4x64 key
);

bool randompack_uint64_philox( // Counter based random number generation with "philox"
  uint64_t x[],             // out  len-vector of integers (unbounded)
  size_t len,               // in   number requested
  randompack_counter ctr,   // in   counter state
  randompack_philox_key key // in   threefry4x64 key
);

bool randompack_serialize( // Serialize an RNG to an opaque byte buffer
  uint8_t *buf,          // out     buffer for serialization (may be NULL if *len==0)
  int *len,              // in/out  0 → query size; otherwise buffer length
  randompack_rng *rng    // in      RNG to serialize
);

bool randompack_deserialize( // Restore an RNG from an opaque byte buffer
  uint8_t *buf,          // in      buffer with serialization
  int len,               // in      buffer length
  randompack_rng *rng    // out     target RNG (must be allocated)
);

bool randompack_set_state( // Set engine state array directly
  uint64_t state[],       // in      state words (length depends on engine)
  int nstate,             // in      number of state words provided
  randompack_rng *rng     // in/out  target RNG
);

//========================================================================================
// Single precision (float) versions of the randompack distributions
//========================================================================================

bool randompack_u01f( // Generate uniform random floats in [0,1), false on error
  float x[],            // out     n-vector: uniform random numbers in [0,1)
  size_t n,             // in      Number of variates
  randompack_rng *rng   // in/out  Random number generator
);

#endif /* RANDOMPACK_H */
