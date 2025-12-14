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
  int len,              // in      Number of integers requested
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

bool randompack_u01( // Generate uniform random numbers in [0,1), false on error
  double x[],           // out     n-vector: uniform random numbers in [0,1)
  int n,                // in      Number of variates
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_norm( // Generate standard normal random numbers N(0,1), false on error
  double x[],           // out     n-vector: standard normal random numbers
  int n,                // in      Number of variates
  randompack_rng *rng   // in/out  Random number generator
);

bool randompack_mvn( // Generate multivariate normal randoms N(mu,Sig), false on error
  char *transp,         // in      "N" to get n×d X, "T" to get d×n X
  double mu[],          // in      d-vector: mean (NULL → zero-mean)
  double Sig[],         // in      d×d covariance matrix (NULL → use L as-is)
  int d,                // in      Dimension of each vector
  int n,                // in      Number of replicates
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

// bool randompack_split( // Generate a mixed hash of seed and path (index array)
//   uint64_t seed,         // in    Seed to use
//   uint64_t *path,        // in    Array of..
//   int *child_seeds,      // out   
//   int nchildren
// );

//========================================================================================
// Advanced API: Low-level utilities and engine-specific features
//
// These functions expose additional control over RNG behaviour, distribution kernels, and
// bit-precise integer generation. They are intended for specialised use cases, testing,
// and performance tuning, and are typically not needed in routine applications.
//========================================================================================

bool randompack_uint32( // Generate uint32 in [0, bound), false on error
  uint32_t x[],          // out     len-vector of integers
  int len,               // in      number requested
  uint32_t bound,        // in      exclusive upper bound, or 0 for unbounded
  randompack_rng *rng    // in/out  random number generator
);

bool randompack_uint64( // Generate uint64 in [0, bound), false on error
  uint64_t x[],          // out     len-vector of integers
  int len,               // in      number requested
  uint64_t bound,        // in      exclusive upper bound, or 0 for unbounded
  randompack_rng *rng    // in/out  random number generator
);

bool randompack_uint64_3fry( // Counter based random number generation with "threefry"
  uint64_t x[],             // out  len-vector of integers (unbounded)
  int len,                  // in   number requested
  randompack_counter ctr,   // in   counter state
  randompack_3fry_key key   // in   threefry4x64 key
);

bool randompack_uint64_philox( // Counter based random number generation with "philox"
  uint64_t x[],             // out  len-vector of integers (unbounded)
  int len,                  // in   number requested
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

// bool randompack_set_state_array( //  TODO implement this
//   uint64_t *state;
//   int len;
//   randompack_rng *rng
// };

bool randompack_set_norm_method( // Set algorithm used for random normals
  char *method,          // in      "polar" or "default" (for ziggurat)
  randompack_rng *rng    // in      Random number generator
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
