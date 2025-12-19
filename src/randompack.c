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
#include "pcg64.h"
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
typedef void (*engine_fill)(uint64_t *out, size_t n, randompack_rng *rng);

struct randompack_rng {
  union {
    uint32_t u32;
    uint64_t u64[4];
    #ifdef HAVE128
    pcg64_t pcg;   // <--- add this
    #endif
  } state;
  rng_engine engine;
  int buf_word;
  int buf_byte;
  engine_next next;
  engine_fill fill;
  double spare_norm;
  char *last_error;
  uint64_t *extra_state;
  uint64_t buf[BUFFSIZE];
};

typedef struct {
  char *full;
  char *abbrev;
  rng_engine  engine;
  int state_words;
  int extra_words;
  engine_next next;
  engine_fill fill;
} rng_entry;

#define ROTL64(x,k) (((x) << (k)) | ((x) >> (64 - (k))))
static uint64_t nextss_ref(randompack_rng *rng) {
  uint64_t *s = rng->state.u64;
  uint64_t result = ROTL64(s[1] * 5, 7) * 9;
  uint64_t t = s[1] << 17;
  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];
  s[2] ^= t;
  s[3] = ROTL64(s[3], 45);
  return result;
}

#include "engines.inc"
#include "buffer_draw.inc"
#include "randutil.inc"
#include "distributions.inc"

#ifdef HAVE128 // Thin wrapper
static uint64_t next_pcg64(randompack_rng *rng) {
  return pcg64_random_fast(&rng->state.pcg);
}
#else
#define next_pcg64 0
#endif

static rng_entry rng_table[] = {
  { "park-miller",   "pm",       PARKMILLER, 1, 0,  0,            0             },
  { "xorshift128+",  "x128+",    X128P,      2, 0,  xorshift128p, fill64_generic},
  { "xoshiro256**",  "x256**",   X256SS,     4, 0,  nextss_ref,   fill64_x256ss },
  { "xoshiro256++",  "x256++",   X256PP,     4, 0,  nextpp,       fill64_generic},
  { "pcg64_dxsm",    "pcg64",    PCG64,      4, 0,  next_pcg64,   fill64_generic},
  { "philox",        "philox",   PHILOX,     6, 7,  next_philox,  fill64_generic},
  { "chacha20",      "chacha20", CHACHA20,   6, 17, chachacha,    fill64_generic},
  { "system-csprng", "system",   SYS,        0, 0,  csprng,       fill64_generic}
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
    rng->fill   = fill64_generic;
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
		rng->fill   = rng_table[i].fill;
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
    rng->buf_word = BUFFSIZE;
    rng->buf_byte = 0;
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
  rng->buf_word = BUFFSIZE;
  rng->buf_byte = 0;
  rng->spare_norm = INFINITY;
  return true;
}

randompack_rng *randompack_create(const char *engine) {
  randompack_rng *rng;
  // Create engine
  if (!ALLOC(rng, 1)) return 0;
  rng->last_error = 0;
  rng->spare_norm = INFINITY; // Use ziggurat iff INFINITY
  rng->engine = INVALID;
  rng->buf_word = BUFFSIZE;
  rng->buf_byte = 0;
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
    rng->buf_word = BUFFSIZE;
    rng->buf_byte = 0;
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
  double spare_norm;
  uint32_t buf_word;
  uint32_t buf_byte;
  uint32_t reserved0;
  uint32_t reserved1;
  ChaCha20_Ctx chacha;
} rng_blob;

enum {
  STATE_MIN_NEED =
  sizeof(uint32_t)*2
  + sizeof(((rng_blob *)0)->state_u64)
  + sizeof(double)
  + sizeof(uint32_t)*4
};

#include "serializations.inc"

bool randompack_serialize(uint8_t *buf, int *len, randompack_rng *rng) {
  // Returns the complete internal state of rng as an opaque byte buffer
  if (!rng) return false;
  rng->last_error = 0;
  if (rng->engine == SYS) {
    rng->last_error = "randompack_serialize: system-csprng not supported";
    return false;
  }
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
  if (rng->engine == SYS) {
    rng->last_error = "randompack_deserialize: system-csprng not supported";
    return false;
  }
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
  if (blob.engine == SYS) {
    rng->last_error = "randompack_deserialize: system-csprng not supported";
    return false;
  }
  if (rng->engine != INVALID && rng->engine != (rng_engine)blob.engine) {
    rng->last_error = "randompack_deserialize: engine mismatch";
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

static bool allzero64(uint64_t *x, int n) {
  for (int i = 0; i < n; i++) if (x[i] != 0) return false;
  return true;
}

bool randompack_set_state(uint64_t state[], int nstate, randompack_rng *rng) {
  if (!rng) return false;
  rng->last_error = 0;
  if (!state || nstate < 0) {
    rng->last_error = "randompack_set_state: invalid arguments";
    return false;
  }
  rng_entry *ent = find_entry(rng->engine);
  assert(ent);
  if (rng->engine == SYS) {
    rng->last_error = "randompack_set_state: not supported for system-csprng";
    return false;
  }
  if (rng->engine == PCG64 && !ent->next) {
    rng->last_error =
      "randompack_set_state: PCG64 engine not supported on this platform";
    return false;
  }
  if (nstate != ent->state_words) {
    rng->last_error = "randompack_set_state: wrong nstate for this engine";
    return false;
  }
  if ((rng->engine == X256SS || rng->engine == X256PP) && allzero64(state, 4)) {
    rng->last_error = "randompack_set_state: xoshiro state must be nonzero";
    return false;
  }
  if (rng->engine == PCG64 && (state[2] & 1) == 0) {
    rng->last_error = "randompack_set_state: pcg64 increment must be odd";
    return false;
  }
  if (rng->engine == PARKMILLER && (state[0] < 1 || state[0] >= (uint64_t)mersenne8)) {
    rng->last_error = "randompack_set_state: Park-Miller state out of range";
    return false;
  }
  set_state(state, nstate, rng);
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

bool randompack_u01(double x[], size_t len, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || len < 0)
    rng->last_error = "invalid arguments to randompack_u01";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  
  rand_dble(x, len, rng);
  return true;
}

bool randompack_int(int x[], size_t len, int m, int n, randompack_rng *rng) {
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
  for (size_t i = 0; i < len; i++) x[i] += m;
  return true;
}

bool randompack_uint32(uint32_t x[], size_t len, uint32_t bound, randompack_rng *rng) {
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

bool randompack_uint16(uint16_t x[], size_t len, uint16_t bound, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || len < 0)
    rng->last_error = "randompack_uint16: invalid arguments";
  else if (rng->engine == PARKMILLER)
    rng->last_error = "randompack_uint16: Park-Miller does not support uint16 randoms";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;  
  rand_uint16(bound, x, len, rng);
  return true;
}

bool randompack_uint8(uint8_t x[], size_t len, uint8_t bound, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || len < 0)
    rng->last_error = "randompack_uint8: invalid arguments";
  else if (rng->engine == PARKMILLER)
    rng->last_error = "randompack_uint8: Park-Miller does not support uint8 randoms";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;  
  rand_uint8(bound, x, len, rng);
  return true;
}

bool randompack_uint64(uint64_t x[], size_t len, uint64_t bound, randompack_rng *rng) {
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

bool randompack_uint64_3fry(uint64_t x[], size_t len, randompack_counter ctr,
                            randompack_3fry_key key) {
  if (!x || len < 0) return false;
  rand_3fry_64bits(x, len, ctr, key);
  return true;
}

bool randompack_uint64_philox(uint64_t x[], size_t len, randompack_counter ctr,
                              randompack_philox_key key) {
  if (!x || len < 0) return false;
  rand_phil_64bits(x, len, ctr, key);
  return true;
}

bool randompack_norm(double x[], size_t len, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || len < 0)
    rng->last_error = "invalid arguments to randompack_norm";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  rand_normal(x, len, rng);
  return true;
}

bool randompack_mvn(char *transp, double mu[], double Sig[], int d, size_t n, double X[],
                    int ldX, double L[], randompack_rng *rng) {
  if (!rng) return false;
  if (!Sig && !L)
    rng->last_error = "randompack_mvn: either Sig or L must be specified";
  else if (!transp || (transp[0] != 'N' && transp[0] != 'T'))
    rng->last_error = "randompack_mvn: transp must begin with 'N' or 'T'";
  else if ((!X && n > 0) || d <= 0 || n < 0 || (ldX <= 0 && X))
    rng->last_error = "randompack_mvn: invalid arguments";
  else if (X && n > 0 && (size_t)ldX < ((transp[0] == 'N') ? n : d))
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
