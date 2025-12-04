// RandomNumbers — random number generation utilities for VARMASIM
//
// See RandomNumbers.h for further information, including parameter descriptions

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "randompack.h"
#include "randompack_config.h"
#include "BlasGateway.h"
#include "crypto_random.inc"

typedef struct randompack_rng randompack_rng;

typedef enum {
  INVALID,
  PARKMILLER,
  X128P,
  X256SS,
  X256PP,
  PCG64,
  CHACHA20,
  SYS,
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

static rng_entry *find_entry(rng_engine e) {
  for (int i = 0; i < LEN(rng_table); i++)
    if (rng_table[i].engine == e) return &rng_table[i];
  return 0;
}

static bool select_engine(const char *s, randompack_rng *rng) {
  // set rng->{engine,next} according to the engine name s
  if (!rng) return false;
  if (!s) {
    rng->engine = X256PP; // default engine
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

static inline bool rng_ok(randompack_rng *rng) {
  if (!rng || rng->engine == INVALID) {
    if (rng) rng->last_error = "invalid randompack_rng object";
    return false;
  }
  return true;
}

randompack_rng *randompack_create(const char *engine, uint64_t seed) {
  randompack_rng *rng;
  ALLOC(rng, 1);
  if (!rng) return 0;
  rng->last_error = 0;
  rng->spare_norm = INFINITY; // Use ziggurat iff INFINITY
  rng->engine = INVALID;
  if (!select_engine(engine, rng)) {
    rng->engine = INVALID;
    rng->last_error = "unknown engine name (spelling error in requested engine)";
    return rng;
  }
  if (rng->engine == PCG64 && !rng->next) {
    rng->last_error = "PCG64 engine not supported on this platform (no 128-bit integers)";
    return rng;
  }
  if (rng->engine == CHACHA20) {
    ChaCha20_Ctx *ctx;
    if (!ALLOC(ctx, 1)) {
      rng->last_error = "randompack_create: memory allocation failed";
      return rng;
    }
    rng->extra_state = ctx;
  }
  if (rng->engine == PARKMILLER) {
    uint64_t x;
    uint32_t s;
    if (seed == 0) {
      entropy_fill(rng, &x, sizeof x);
      s = (uint32_t)(x % (uint32_t)mersenne8);
    }
    else
      s = (uint32_t)(seed % (uint64_t)mersenne8);
    if (s == 0) s = 1;
    rng->state.u32 = s;
    rng->buf64 = 0;
    (void)PM_rand_bits(rng); // spin-up
    return rng;
  }
  if (seed == 0)
    rand_randomize(rng);
  else {
    if (rng->engine == CHACHA20) {
      key256_t key;
      nonce96_t nonce;
      ChaCha20_Ctx *ctx = (ChaCha20_Ctx *)rng->extra_state;
      for (unsigned int i = 0; i < sizeof(key)/sizeof(key[0]); i++)
        key[i] = (uint8_t)rand_splitmix64(&seed);
      for (unsigned int i = 0; i < sizeof(nonce)/sizeof(nonce[0]); i++)
        nonce[i] = (uint8_t)rand_splitmix64(&seed);
      ChaCha20_init(ctx, key, nonce, 0);
    }
    else {
      for (int i = 0; i < LEN(rng->state.u64); i++)
        rng->state.u64[i] = rand_splitmix64(&seed);
      if (rng->state.u64[0] == 0) rng->state.u64[0] = 1;
    }
  }
  return rng;
}

void randompack_free(randompack_rng *rng) {
  if (!rng) return;
  FREE(rng->extra_state);
  FREE(rng);
}

typedef struct {
  uint32_t version;
  uint32_t engine;
  uint64_t state_u64[4];
  uint64_t buf64;
  double spare_norm;
  uint32_t reserved;
  ChaCha20_Ctx chacha;
} rng_blob;

enum {
  STATE_MIN_NEED =
  sizeof(uint32_t)*2
  + sizeof(((rng_blob *)0)->state_u64)
  + sizeof(uint64_t)
  + sizeof(double)
  + sizeof(uint32_t)*2
};

bool randompack_get_state(int *len, uint8_t *buf, randompack_rng *rng) {
  // Returns the complete internal state of rng as an opaque byte buffer
  if (!rng) return false;
  if (!len) {
    rng->last_error = "randompack_get_state: len is null";
    return false;
  }
  int need = STATE_MIN_NEED + (rng->engine == CHACHA20 ? sizeof(ChaCha20_Ctx) : 0);
  if (!buf) {
    *len = need;
    rng->last_error = 0;
    return true;
  }
  if (*len < need) {
    rng->last_error = "randompack_get_state: buffer too small";
    return false;
  }
  rng_blob blob = {0};
  blob.version = 1;
  blob.engine = rng->engine;
  for (int i=0; i<LEN(rng->state.u64); i++) blob.state_u64[i] = rng->state.u64[i];
  blob.buf64 = rng->buf64;
  blob.spare_norm = rng->spare_norm;
  if (rng->engine==CHACHA20 && rng->extra_state)
    memcpy(&blob.chacha, rng->extra_state, sizeof(ChaCha20_Ctx));
  memcpy(buf, &blob, need);
  rng->last_error = 0;
  return true;
}

bool randompack_set_state(int len, const uint8_t *buf, randompack_rng *rng) {
  // Restores the rng state using a buffer obtained with randompack_get_state
  if (!rng) return false;
  if (!buf || len <= 0) goto invalid_args;

  rng_blob blob = {0};
  memcpy(&blob, buf, imin(len, sizeof(blob)));

  rng_entry *ent = find_entry(blob.engine);
  int extra_need = (blob.engine == CHACHA20 ? sizeof(ChaCha20_Ctx) : 0);
  if (blob.version != 1
      || !ent
      || len < STATE_MIN_NEED + extra_need)
    goto corrupt;  
  
  rng->engine = blob.engine;
  rng->next   = ent->next;

  if (rng->engine == CHACHA20) {
    if (!rng->extra_state) {
      ChaCha20_Ctx *ctx;
      if (!ALLOC(ctx, 1)) goto alloc_fail;
      rng->extra_state = ctx;
    }
    memcpy(rng->extra_state, &blob.chacha, sizeof(ChaCha20_Ctx));
  }
  for (int i = 0; i < LEN(rng->state.u64); i++)
    rng->state.u64[i] = blob.state_u64[i];
  rng->state.u32 = blob.state_u64[0];
  rng->buf64 = blob.buf64;
  rng->spare_norm = blob.spare_norm;
  rng->last_error = 0;
  return true;
  
invalid_args:
  rng->last_error = "randompack_set_state: invalid arguments";
  return false;
corrupt:
  rng->last_error = "randompack_set_state: corrupt state buffer";
  return false;
alloc_fail:
  rng->last_error = "randompack_set_state: allocation failed";
  return false;
}

const char *randompack_last_error(const randompack_rng *rng) {
  if (!rng) return 0;
  return rng->last_error;
}

bool randompack_set_norm_method(char *method, randompack_rng *rng) {
  if (!rng) return false;
  rng->last_error = 0;
  char t[10];
  STRSET(t, method);
  for (int i=0; t[i]; i++) t[i] = TOLOWER(t[i]);
  if (!strcmp(method, "polar"))
	 rng->spare_norm = NAN;  // Codes polar
  else if (!strcmp(method, "default"))
	 rng->spare_norm = INFINITY;
  else
	 rng->last_error = "randompack_set_norm_method: invalid method argument";
  if (rng->last_error) return false;
  return true;
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

bool randompack_uint64_3fry(uint64_t x[], int len, randompack_counter ctr,
                            randompack_3fry_key key) {
  if (!x || len < 0)
    return false;
  rand_3fry_64bits(x, len, ctr, key);
  return true;
}

bool randompack_uint64_philox(uint64_t x[], int len, randompack_counter ctr,
                              randompack_philox_key key) {
  if (!x || len < 0)
    return false;
  rand_phil_64bits(x, len, ctr, key);
  return true;
}

bool randompack_norm(double x[], int len, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || len < 0)
    rng->last_error = "invalid arguments to randompack_norm";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  rand_normal(x, len, rng);
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
