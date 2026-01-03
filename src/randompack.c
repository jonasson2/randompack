// RandomNumbers — random number generation utilities for VARMASIM
//
// See RandomNumbers.h for further information, including parameter descriptions

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "randompack.h"
#include "randompack_config.h"
#include "BlasGateway.h"
#include "cwg128_64.h"
#include "pcg64.h"
#include "crypto_random.inc"

typedef struct randompack_rng randompack_rng;

typedef enum {
  INVALID,
  X128P,
  XORO,
  X256SS,
  X256PP,
  SQUARES,
  PCG64,
  PHILOX,
  CHACHA20,
  SYS,
  CWG128,
} rng_engine;

typedef void (*engine_fill)(randompack_rng *rng);

struct randompack_rng {
  union {
	 uint8_t  u8[48];
    uint32_t u32[16];
    uint64_t u64[8];
    #ifdef HAVE128
    pcg64_t pcg;
    cwg128_64_t cwg;
    #endif
  } state;
  rng_engine engine;
  int buf_word32;
  int buf_word;
  int buf_byte;
  engine_fill fill;
  char *last_error;
  union {
    uint32_t u32[2*BUFSIZE];
    uint64_t u64[BUFSIZE];
  } buf;
};

typedef struct {
  char *full;
  char *abbrev;
  rng_engine  engine;
  int state_words;
  engine_fill fill;
} rng_entry;

#include "engines.inc"
#include "buffer_draw.inc"
#include "randutil.inc"
#include "distributions.inc"

#ifndef HAVE128
#define next_pcg64 0
#endif

static rng_entry rng_table[] = {
  { "xorshift128+",  "x128+",     X128P,      2, fill_x128p     },
  { "xoroshiro128++","xoro++",    XORO,       2, fill_xoro128pp },
  { "xoshiro256**",  "x256**",    X256SS,     4, fill_x256ss    },
  { "xoshiro256++",  "x256++",    X256PP,     4, fill_x256pp    },
  { "squares64",     "squares",   SQUARES,    2, fill_squares   },
  { "pcg64_dxsm",    "pcg64",     PCG64,      4, fill_pcg64     },
  { "cwg128_64",     "cwg128",    CWG128,     5, fill_cwg128    },
  { "philox",        "philox",    PHILOX,     6, fill_philox    },
  { "chacha20",      "chacha20",  CHACHA20,   6, fill_chacha    },
  { "system-csprng", "system",    SYS,        0, fill_csprng    }
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
    rng->fill   = fill_x256pp;
    return true;
  }
  char t[64];
  STRSET(t, s);
  for (int i = 0; t[i]; i++) t[i] = TOLOWER(t[i]);
  for (int i = 0; i < LEN(rng_table); i++) {
    if (!strcmp(t, rng_table[i].full) ||
        !strcmp(t, rng_table[i].abbrev)) {
    rng->engine = rng_table[i].engine;
    rng->fill   = rng_table[i].fill;
     return true;
    }
  }
  return false;  // unknown engine
}

bool randompack_seed(int seed, uint32_t *spawn_key, int nkey, randompack_rng *rng)  {
  if (!rng) return false;
  uint32_t seed32 = seed;
  rng->last_error = 0;
  // Use Melissa O'Neill's seed sequence
  if (nkey < 0 || (nkey > 0 && !spawn_key)) {
    rng->last_error = "randompack_seed: invalid spawn_key arguments";
    return false;
  }
  uint32_t w[16];
  bool ok = seed_seq_seed(w, 16, seed32, spawn_key, nkey);
  if (!ok) {
    rng->last_error = "randompack_seed: allocation failed";
    return false;
  }
  rng_entry *ent = find_entry(rng->engine);
  copy32(rng->state.u32, w, ent->state_words*2);
  if (rng->state.u64[0] == 0) { // the xo-family needs a nonzero state
    rng->state.u64[0] = 1;
  }
  rng->buf_word32 = -1;
  rng->buf_word = BUFSIZE;
  rng->buf_byte = 0;
  return true;
}

randompack_rng *randompack_create(const char *engine) {
  randompack_rng *rng;
  #if defined(RANDOMPACK_NEED_RUNTIME_ENDIAN_CHECK)
  uint32_t endian = 1;
  if (*(uint8_t *)&endian != 1) {
    fputs("randompack: big-endian platforms are not supported\n", stderr);
    abort();
  }
  #endif
  // Create engine
  if (!ALLOC(rng, 1)) return 0;
  rng->last_error = 0;
  rng->engine = INVALID;
  rng->buf_word32 = -1;
  rng->buf_word = BUFSIZE;
  rng->buf_byte = 0;
  if (!select_engine(engine, rng)) {
    rng->last_error = "unknown engine name (spelling error in requested engine)";
    return rng;
  }
  if (rng->engine == PCG64 && !HAVE128) {
    rng->last_error = "PCG64 engine not supported on this platform (no 128-bit integers)";
    return rng;
  }
  if (rng->engine == CWG128 && !HAVE128) {
    rng->last_error =
      "CWG128 engine not supported on this platform (no 128-bit integers)";
    return rng;
  }
  rng_entry *ent = find_entry(rng->engine);
  (void)ent;
  rand_randomize(rng);
  return rng;
}

void randompack_free(randompack_rng *rng) {
  if (!rng) return;
  FREE(rng);
}

typedef struct {
  uint32_t version;
  uint32_t engine;
  uint64_t state_u64[4];
  uint32_t buf_word;
  uint32_t buf_byte;
  uint32_t reserved0;
  uint32_t reserved1;
  uint64_t buf[BUFSIZE];
} rng_blob;

enum {
  STATE_NEED =
  sizeof(uint32_t)*2
  + sizeof(((rng_blob *)0)->state_u64)
  + sizeof(uint32_t)*4
  + sizeof(((rng_blob *)0)->buf)
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
  int need = STATE_NEED;
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
  memcpy(&blob, buf, min(len, STATE_NEED));
  rng_entry *ent = find_entry(blob.engine);
  int need = STATE_NEED;
  if (blob.version != 1 || !ent || len < need) {
    rng->last_error = "randompack_deserialize: corrupt state buffer";
    return false;
  }
  if (blob.engine == SYS) {
    rng->last_error = "randompack_deserialize: system-csprng not supported";
    return false;
  }
  if (rng->engine != INVALID && rng->engine != blob.engine) {
    rng->last_error = "randompack_deserialize: engine mismatch";
    return false;
  }
  if (blob.engine == PCG64 && !HAVE128) {
    rng->last_error =
      "randompack_deserialize: PCG64 engine not supported on this platform";
    return false;
  }
  if (blob.engine == CWG128 && !HAVE128) {
    rng->last_error =
      "randompack_deserialize: CWG128 engine not supported on this platform";
    return false;
  }
  bool ok = deserialize(&blob, ent, rng);
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
  if (rng->engine == SYS)
    rng->last_error = "randompack_set_state: not supported for system-csprng";
  else if (rng->engine == PCG64 && !HAVE128)
    rng->last_error =
      "randompack_set_state: PCG64 engine not supported on this platform";
  else if (rng->engine == CWG128 && !HAVE128)
    rng->last_error =
      "randompack_set_state: CWG128 engine not supported on this platform";
  else if (nstate != ent->state_words)
    rng->last_error = "randompack_set_state: wrong nstate for this engine";
  else if ((rng->engine == X256SS || rng->engine == X256PP || rng->engine == XORO ||
				rng->engine == X128P ) && allzero64(state, 4))
    rng->last_error = "randompack_set_state: xoshiro state must be nonzero";
  else if (rng->engine == PCG64 && (state[2] & 1) == 0)
    rng->last_error = "randompack_set_state: pcg64 increment must be odd";
  else if (rng->engine == CWG128 && (state[4] & 1) == 0)
    rng->last_error = "randompack_set_state: cwg128 increment must be odd";
  if (rng->last_error) return false;
  set_state(state, nstate, rng);
  return true;
}

#ifdef HAVE128
bool randompack_pcg64_set_state(uint128_t state, uint128_t inc, randompack_rng *rng) {
  if (!rng) return false;
  rng->last_error = 0;
  if (rng->engine != PCG64) {
    rng->last_error = "randompack_pcg64_set_state: engine is not pcg64";
    return false;
  }
  pcg64_set_state(state, inc, rng);
  return true;
}
#endif

bool randompack_philox_set_state(randompack_counter ctr, randompack_philox_key key,
  randompack_rng *rng) {
  if (!rng) return false;
  rng->last_error = 0;
  if (rng->engine != PHILOX) {
    rng->last_error = "randompack_philox_set_state: engine is not philox";
    return false;
  }
  philox_set_state(ctr, key, rng);
  return true;
}

bool randompack_squares_set_state(uint64_t ctr, uint64_t key,
  randompack_rng *rng) {
  if (!rng) return false;
  rng->last_error = 0;
  if (rng->engine != SQUARES) {
    rng->last_error = "randompack_squares_set_state: engine is not squares64";
    return false;
  }
  squares_set_state(ctr, key, rng);
  return true;
}

char *randompack_last_error(randompack_rng *rng) {
  if (!rng) return 0;
  return rng->last_error;
}

double randompack_u01_draw(randompack_rng *rng) {
  return (draw_u64(rng) >> 11) * 0x1.0p-53;
}

bool randompack_u01(double x[], size_t len, randompack_rng *rng) {
  if (!rng) return false;
  if (!x)
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
  if (!x)
    rng->last_error = "invalid arguments to randompack_int";
  else if (m > n)
    rng->last_error = "randompack_int: m must be <= n";
  else if (span > INT_MAX)
    rng->last_error = "randompack_int: n - m must be < 2^31";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  
  rand_uint32((uint32_t*)x, len, span + 1, rng);
  for (size_t i = 0; i < len; i++) x[i] += m;
  return true;
}

bool randompack_uint8(uint8_t x[], size_t len, uint8_t bound, randompack_rng *rng) {
  if (!rng) return false;
  if (!x)
    rng->last_error = "randompack_uint8: invalid arguments";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;  
  rand_uint8(x, len, bound, rng);
  return true;
}

bool randompack_uint16(uint16_t x[], size_t len, uint16_t bound, randompack_rng *rng) {
  if (!rng) return false;
  if (!x)
    rng->last_error = "randompack_uint16: invalid arguments";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;  
  rand_uint16(x, len, bound, rng);
  return true;
}

bool randompack_uint32(uint32_t x[], size_t len, uint32_t bound, randompack_rng *rng) {
  if (!rng) return false;
  if (!x)
    rng->last_error = "randompack_uint32: invalid arguments";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;  
  rand_uint32(x, len, bound, rng);
  return true;
}

bool randompack_uint64(uint64_t x[], size_t len, uint64_t bound, randompack_rng *rng) {
  if (!rng) return false;
  if (!x)
    rng->last_error = "invalid arguments to randompack_uint64";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;

  rand_uint64(x, len, bound, rng);
  return true;
}

bool randompack_perm(int x[], int len, randompack_rng *rng) {
  if (!rng) return false;
  if (!x)
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
  if (!x || k < 0 || k > len)
    rng->last_error = "invalid arguments to randompack_sample";
  else if (len > INT_MAX - 1)
    rng->last_error = "randompack_sample: len must be <= 2^31 - 2";	 
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  rand_sample(x, len, k, rng);
  return true;
}

bool randompack_unif(double x[], size_t len, double a, double b,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || !(a < b)) {
    rng->last_error = "invalid arguments to randompack_unif";
    return false;
  }
  rng->last_error = 0;
  rand_dble(x, len, rng); // x in [0,1)
  double w = b - a;
  for (size_t i = 0; i < len; i++) x[i] = fmin(b, a + w*x[i]);
  return true;
}

bool randompack_norm(double x[], size_t len, randompack_rng *rng) { // standard normal
  if (!rng) return false;
  if (!x)
    rng->last_error = "invalid arguments to randompack_norm";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  rand_norm(x, len, rng);
  // rand_norm(x, len, rng);
  return true;
}

bool randompack_normal(double x[], size_t len, double mu, double sigma, randompack_rng
							  *rng) { // general normal
  if (!rng) return false;
  if (!x || sigma <= 0) {
    rng->last_error = "invalid arguments to randompack_normal";
    return false;
  }
  rng->last_error = 0;
  rand_norm(x, len, rng);     // generate N(0,1)
  if (mu != 0 || sigma != 1)
    for (size_t i = 0; i < len; i++) x[i] = mu + sigma*x[i];
  return true;
}

bool randompack_lognormal(double x[], size_t len, double mu, double sigma,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || sigma <= 0) {
    rng->last_error = "invalid arguments to randompack_lognormal";
    return false;
  }
  rng->last_error = 0;
  rand_norm(x, len, rng); // N(0,1)
  for (size_t i = 0; i < len; i++) {
    x[i] = exp(mu + sigma*x[i]);
  }
  return true;
}

bool randompack_gumbel(double x[], size_t len, double mu, double beta,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || beta <= 0.0) {
    rng->last_error = "invalid arguments to randompack_gumbel";
    return false;
  }
  rng->last_error = 0;
  rand_dble(x, len, rng); // x in [0,1)
  for (size_t i=0; i<len; i++)
    x[i] = mu - beta*log(-log(x[i]));
  return true;
}

bool randompack_pareto(double x[], size_t len, double xm, double alpha, randompack_rng
                       *rng) {
  if (!rng) return false;
  if (!x || xm <= 0 || alpha <= 0) {
    rng->last_error = "invalid arguments to randompack_pareto";
    return false;
  }
  rng->last_error = 0;
  rand_exp(x, len, rng);
  for (size_t i = 0; i < len; i++)
    x[i] = xm*exp(x[i]/alpha);
  return true;
}

bool randompack_exp(double x[], size_t len, double scale, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || scale <= 0) {
    rng->last_error = "invalid arguments to randompack_exp";
	 return false;
  }
  rng->last_error = 0;
  rand_exp(x, len, rng);
  if (scale != 1) for (size_t i = 0; i < len; i++) x[i] *= scale;
  return true;
}

bool randompack_gamma(double x[], size_t len, double shape, double scale,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || shape <= 0 || scale <= 0) {
    rng->last_error = "invalid arguments to randompack_gamma";
    return false;
  }
  rng->last_error = 0;
  fill_gamma(x, len, shape, scale, rng);
  return true;
}

bool randompack_chi2(double x[], size_t len, double nu, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || nu <= 0) {
    rng->last_error = "invalid arguments to randompack_chi2";
    return false;
  }
  rng->last_error = 0;
  fill_gamma(x, len, 0.5*nu, 2, rng);
  return true;
}

bool randompack_beta(double x[], size_t len, double a, double b,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || a <= 0 || b <= 0) {
    rng->last_error = "invalid arguments to randompack_beta";
    return false;
  }
  rng->last_error = 0;
  fill_beta(x, len, a, b, rng);
  return true;
}

bool randompack_t(double x[], size_t len, double nu, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || nu <= 0) {
    rng->last_error = "invalid arguments to randompack_t";
    return false;
  }
  rng->last_error = 0;
  fill_t(x, len, nu, rng);
  return true;
}

bool randompack_f(double x[], size_t len, double nu1, double nu2,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || nu1 <= 0 || nu2 <= 0) {
    rng->last_error = "invalid arguments to randompack_f";
    return false;
  }
  rng->last_error = 0;
  fill_f(x, len, nu1, nu2, rng);
  return true;
}

// -*- C -*-
bool randompack_weibull(double x[], size_t len, double shape, double scale,
  randompack_rng *rng) {
  if (!rng)
    return false;
  if (!x || shape <= 0 || scale <= 0) {
    rng->last_error = "invalid arguments to randompack_weibull";
    return false;
  }
  rng->last_error = 0;
  rand_exp(x, len, rng); // x[i] = E ~ Exp(1) using ziggurat exponential
  double inv_shape = 1/shape;
  for (size_t i = 0; i < len; i++)
    x[i] = scale*pow(x[i], inv_shape);
  return true;
}

bool randompack_mvn(char *transp, double mu[], double Sig[], int d, size_t n, double X[],
                    int ldX, double L[], randompack_rng *rng) {
  if (!rng) return false;
  if (!Sig && !L)
    rng->last_error = "randompack_mvn: either Sig or L must be specified";
  else if (!transp || (transp[0] != 'N' && transp[0] != 'T'))
    rng->last_error = "randompack_mvn: transp must begin with 'N' or 'T'";
  else if ((!X && n > 0) || d <= 0 || (ldX <= 0 && X))
    rng->last_error = "randompack_mvn: invalid arguments";
  else if (X && n > 0 && (size_t)ldX <
           ((transp[0] == 'N') ? n : (size_t)d))
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

// ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
// Include file with single precision (float) random generators
// ––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
#include "randompack_float.inc"
