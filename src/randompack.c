// RandomNumbers — random number generation utilities for VARMASIM
//
// See RandomNumbers.h for further information, including parameter descriptions

#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <assert.h>
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
  PHILOX,
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
  char *last_error;
  uint64_t *extra_state;
};

typedef struct {
  char *full;
  char *abbrev;
  rng_engine  engine;
  int extra_words;
  engine_next next;
} rng_entry;

#ifndef HAVE128
#define pcg64_random_fast 0
#endif

#include "engines.inc"
#include "randutil.inc"
#include "distributions.inc"

static rng_entry rng_table[] = {
  { "park-miller",   "pm",       PARKMILLER, 0,  0                 },
  { "xorshift128+",  "x128+",    X128P,      0,  xorshift128p      },
  { "xoshiro256**",  "x256**",   X256SS,     0,  nextss            },
  { "xoshiro256++",  "x256++",   X256PP,     0,  nextpp            },
  { "pcg64",         "pcg",      PCG64,      0,  pcg64_random_fast },
  { "philox",        "philox",   PHILOX,     7,  next_philox       },
  { "chacha20",      "chacha20", CHACHA20,   17, chachacha         },
  { "system-csprng", "system",   SYS,        0,  csprng            }
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

bool randompack_seed(int seed, uint32_t *spawn_key, int nkey, randompack_rng *rng)  {
  if (!rng) return false;
  uint32_t seed32 = (uint32_t)seed;
  rng->last_error = 0;
  if (rng->engine == PARKMILLER) {
    if (nkey != 0 || spawn_key != 0) {
      rng->last_error = "randompack_seed: spawn_key not supported for Park-Miller";
      return false;
    }
    uint32_t s = seed32 % mersenne8;
    if (s == 0) s = 1;
    rng->state.u32 = s;
    rng->buf64 = 0;
    (void)PM_rand_bits(rng); // burn-in
  }
  else {  // Use Melissa O'Neill's seed sequence
    if (nkey < 0 || (nkey > 0 && !spawn_key)) {
      rng->last_error = "randompack_seed: invalid spawn_key arguments";
      return false;
    }
    uint32_t w[12];
	 bool ok = seed_seq_seed(w, 12, seed32, spawn_key, nkey);
	 if (!ok) {
		rng->last_error = "randompack_seed: allocation failed";
		return false;
	 }	 
    if (rng->engine == CHACHA20) {
      ChaCha20_Ctx *ctx = (ChaCha20_Ctx *)rng->extra_state;
      key256_t key;
      nonce96_t nonce;
      copy32(key,   w + 0, 8);
      copy32(nonce, w + 8, 3);
      ChaCha20_init(ctx, key, nonce, 0);
    }
    else {
      copy32(rng->state.u64, w, 8);
      if (rng->engine == PHILOX) {
        philox_state *st = (philox_state *)rng->extra_state;
        copy32(&st->key, w + 8, 4);
        st->idx = 4;
      }
      else if (rng->state.u64[0] == 0) { // the xo-family needs a nonzero state
        rng->state.u64[0] = 1;
      }
    }
  }
  return true;
}

randompack_rng *randompack_create(const char *engine) {
  randompack_rng *rng;
  // Create engine
  if (!ALLOC(rng, 1)) return 0;
  rng->last_error = 0;
  rng->spare_norm = INFINITY; // Use ziggurat iff INFINITY
  rng->engine = INVALID;
  if (!select_engine(engine, rng)) {
    rng->last_error = "unknown engine name (spelling error in requested engine)";
    return rng;
  }
  if (rng->engine == PCG64 && !rng->next) {
    rng->last_error = "PCG64 engine not supported on this platform (no 128-bit integers)";
    return rng;
  }
  rng_entry *ent = find_entry(rng->engine);
  assert(ent);
  if (ent->extra_words > 0) {
    uint64_t *p;
    if (!ALLOC(p, ent->extra_words)) {
      rng->last_error = "randompack_create: memory allocation failed";
      return rng;
    }
    rng->extra_state = p;
  }
  if (rng->engine == PARKMILLER) {
    uint64_t x;
    uint32_t s;
    entropy_fill(rng, &x, sizeof x);
    s = (uint32_t)(x % (uint32_t)mersenne8);
    if (s == 0) s = 1;
    rng->state.u32 = s;
    rng->buf64 = 0;
  }
  else {
    rand_randomize(rng);
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

#include "serializations.inc"

bool randompack_serialize(uint8_t *buf, int *len, randompack_rng *rng) {
  // Returns the complete internal state of rng as an opaque byte buffer
  if (!rng) return false;
  rng->last_error = 0;
  if (!len) {
    rng->last_error = "randompack_serialize: len is null";
    return false;
  }
  int need = serialized_need(rng);
  if (!buf) { // Report needed buffer size
    *len = need;
    return true;
  }
  if (*len < need) {
    rng->last_error = "randompack_serialize: buffer too small";
    return false;
  }
  serialize(buf, *len, rng);
  return true;
}

bool randompack_deserialize(uint8_t *buf, int len, randompack_rng *rng) {
  // Restores the rng state using a buffer obtained with randompack_serialize
  if (!rng) return false;
  rng->last_error = 0;
  if (!buf || len <= 0) {
    rng->last_error = "randompack_deserialize: invalid arguments";
    return false;
  }
  rng_blob blob = {0};
  memcpy(&blob, buf, min(len, STATE_MIN_NEED));
  rng_entry *ent = find_entry(blob.engine);
  int need = serialized_need_from_blob(&blob);
  if (blob.version != 1 || !ent || len < need) {
    rng->last_error = "randompack_deserialize: corrupt state buffer";
    return false;
  }
  if (blob.engine == PCG64 && !ent->next) {
    rng->last_error =
      "randompack_deserialize: PCG64 engine not supported on this platform";
    return false;
  }
  int extra = serialized_extra_bytes((rng_engine)blob.engine);
  bool ok = deserialize(&blob, ent, buf + STATE_MIN_NEED, extra, rng);
  if (!ok) {
    rng->last_error = "randompack_deserialize: allocation failed";
    return false;
  }
  return true;
}

char *randompack_last_error(randompack_rng *rng) {
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
  else if (rng->engine == PARKMILLER && span > (int64_t)INT_MAX - 2)
    rng->last_error = "randompack_int: for Park-Miller, n - m must be < 2^31 - 2";
  else if (span > (int64_t)INT_MAX)
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
  else if (len > INT_MAX - 1)
    rng->last_error = "randompack_perm: len must be <= 2^31 - 2";
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
  else if (len > INT_MAX - 1)
    rng->last_error = "randompack_sample: len must be <= 2^31 - 2";	 
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  rand_sample(x, len, k, rng);
  return true;
}

bool randompack_uint64_3fry(uint64_t x[], int len, randompack_counter ctr,
                            randompack_3fry_key key) {
  if (!x || len < 0) return false;
  rand_3fry_64bits(x, len, ctr, key);
  return true;
}

bool randompack_uint64_philox(uint64_t x[], int len, randompack_counter ctr,
                              randompack_philox_key key) {
  if (!x || len < 0) return false;
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
