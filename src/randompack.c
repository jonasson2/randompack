// RandomNumbers — random number generation utilities for VARMASIM
//
// See RandomNumbers.h for further information, including parameter descriptions

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
//#include "randompack.h"
#include "BlasGateway.h"
//#include "VarmaUtilities.h"
#include "randompack_config.h"
#include "crypto_random.inc"

typedef struct randompack_rng randompack_rng;

typedef enum {
  PARKMILLER,
  X128P,
  X256SS,
  X256PP,
  PCG64,
  CHACHA20,
  SYS
} rng_engine;

typedef uint64_t (*engine_next)(randompack_rng *rng);

struct randompack_rng {
  union {
    uint32_t u32;
    uint64_t u64[4];
    #ifdef HAVE128
    uint128_t u128;
    #endif
  } state;
  rng_engine engine;
  engine_next next;
  uint64_t buf64;
  double spare_norm;
  const char *last_error;
  void *extra_state;
};

typedef struct {
  const char *full;
  const char *abbrev;
  rng_engine  engine;
  engine_next next;
} rng_entry;

#ifndef HAVE128
#define pcg64_random_fast 0
#endif

#include "engines.inc"
#include "randutil.inc"
#include "distributions.inc"

static rng_entry rng_table[] = {
  { "park-miller",   "pm",       PARKMILLER,  0                 }, // special path
  { "xorshift128+",  "x128+",    X128P,       xorshift128p      },
  { "xoshiro256**",  "x256**",   X256SS,      nextss            },
  { "xoshiro256++",  "x256++",   X256PP,      nextpp            },
  { "pcg64",         "pcg",      PCG64,       pcg64_random_fast },
  { "chacha20",      "chacha20", CHACHA20,    chachacha         },
  { "system-csprng", "system",   SYS,         csprng            }
};

static bool select_engine(const char *s, randompack_rng *rng) {
  if (!rng) return false;
  if (!s) {
    rng->engine = X256PP;   // default engine
    rng->next   = nextpp;
    return true;
  }
  char t[64];
  STRSET(t, s);
  for (int i = 0; t[i]; i++) t[i] = TOLOWER(t[i]);
  for (int i = 0; i < LEN(rng_table); i++) {
    if (!strcmp(t, rng_table[i].full) ||
        !strcmp(t, rng_table[i].abbrev)) {
      rng->engine = rng_table[i].engine;
      rng->next   = rng_table[i].next;   // may be 0 for PARKMILLER
      return true;
    }
  }
  return false;  // unknown engine
}

randompack_rng *randompack_create(const char *engine, uint64_t seed) {
  randompack_rng *rng;
  ALLOC(rng, 1);
  if (!rng) return 0;
  rng->last_error = 0;
  rng->spare_norm = NAN;
  if (!select_engine(engine, rng)) {
    rng->last_error = "unknown engine name (spelling error in requested engine)";
    return rng;
  }
  if (rng->engine == PCG64 && !rng->next) {
    rng->last_error = "PCG64 engine not supported on this platform (no 128-bit integers)";
    return rng;
  }
  if (seed == 0)
    rand_randomize(rng);
  else {
    for (int i = 0; i < LEN(rng->state.u64); i++)
      rng->state.u64[i] = rand_splitmix64(&seed);
    if (rng->state.u64[0] == 0) rng->state.u64[0] = 1;
  }
  return rng;
}

void randompack_free(randompack_rng *rng) {
  FREE(rng);
}

bool randompack_u01(double x[], int len, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || len < 0)
    rng->last_error = "invalid arguments to randompack_u01";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  
  rand_dble(x, len, rng);
  return true;
}

bool randompack_int(int x[], int len, int m, int n, randompack_rng *rng) {
  if (!rng) return false;

  int64_t span = (int64_t)n - (int64_t)m;

  if (!x || len < 0)
    rng->last_error = "invalid arguments to randompack_int";
  else if (m > n)
    rng->last_error = "randompack_int: m must be <= n";
  else if (rng->engine == PARKMILLER && span > (int64_t)INT32_MAX - 3)
    rng->last_error = "randompack_int: for Park-Miller, n - m must be < 2^31 - 2";
  else if (span > (int64_t)INT32_MAX)
    rng->last_error = "randompack_int: n - m must be < 2^31";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  
  if (rng->engine == PARKMILLER)
    PM_rand_int((int)(span + 1), x, len, rng);
  else
    rand_uint32((uint32_t)(span + 1), (uint32_t*)x, len, rng);
  for (int i = 0; i < len; i++) x[i] += m;
  return true;
}

bool randompack_uint32(uint32_t x[], int len, uint32_t bound, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || len < 0)
    rng->last_error = "randompack_uint32: invalid arguments";
  else if (bound == 0)
    rng->last_error = "randompack_uint32: bound must be > 0";
  else if (rng->engine == PARKMILLER)
    rng->last_error = "randompack_uint32: Park-Miller does not support uint32 randoms";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  
  rand_uint32(bound, x, len, rng);
  return true;
}

bool randompack_uint64(uint64_t x[], int len, uint64_t bound, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || len < 0)
    rng->last_error = "invalid arguments to randompack_uint64";
  else if (bound == 0)
    rng->last_error = "randompack_uint64: bound must be > 0";
  else if (rng->engine == PARKMILLER)
    rng->last_error = "randompack_uint64: Park-Miller does not support uint64 randoms";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;

  rand_uint64(bound, x, len, rng);
  return true;
}

bool randompack_perm(int x[], int len, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || len < 0)
    rng->last_error = "invalid arguments to randompack_perm";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;

  rand_perm(x, len, rng);
  return true;
}

bool randompack_sample(int x[], int len, int k, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || len < 0 || k < 0 || k > len)
    rng->last_error = "invalid arguments to randompack_sample";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;

  rand_sample(x, len, k, rng);
  return true;
}

// #ifdef USING_R
//   if (rng->type == PARKMILLER)
//     rand_normal(x, len, rng);
//   else {
//     GetRNGstate();
//     for (int i = 0; i < len; i++) {
//       x[i] = unif_rand(); // R-s built-in generator
//     }
//     PutRNGstate();
//   }

bool randompack_norm(double x[], int len, randompack_rng *rng) {
  if (!rng) return false;      // cannot set error if rng is NULL
  if (!x || len < 0)
    rng->last_error = "invalid arguments to randompack_norm";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;

  rand_normal(x, len, rng);    // len == 0 is fine
  return true;
}

bool randompack_mvn(char *transp, double mu[], double Sig[], int d, int n, double X[],
                    int ldX, double L[], randompack_rng *rng) {
  if (!rng) return false;
  if (!Sig && !L)
    rng->last_error = "randompack_mvn: either Sig or L must be specified";
  else if (!transp || (transp[0] != 'N' && transp[0] != 'T'))
    rng->last_error = "randompack_mvn: transp must begin with 'N' or 'T'";
  else if ((!X && n > 0) || d <= 0 || n < 0 || (ldX <= 0 && X))
    rng->last_error = "randompack_mvn: invalid arguments";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;

  bool ok = rand_mvn(transp[0], mu, Sig, d, n, X, ldX, L, rng);
  if (!ok) {
    rng->last_error = "randompack_mvn: memory allocation failed";
    return false;
  }
  else return true;
}
