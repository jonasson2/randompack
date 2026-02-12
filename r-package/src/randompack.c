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
#include <limits.h>
#include "randompack.h"
#include "randompack_config.h"
#include "BlasGateway.h"
#if HAVE128
#include "cwg128_64.h"
#include "pcg64.h"
#else
#include "pcg64_emul.h"
#endif
#include "crypto_random.inc"

typedef struct randompack_rng randompack_rng;

typedef enum {
  INVALID,
  X128P,
  XORO,
  X256SS,
  X256PP,
  SQUARES,
  SFC64,
  PCG64,
  PHILOX,
  CHACHA20,
  SYS,
  CWG128,
  FAST,
} rng_engine;

typedef void (*engine_fill)(randompack_rng *rng, size_t len);

typedef struct {
  uint64_t s0[4], s1[4], s2[4], s3[4];
} xo256;

struct randompack_rng {
  union {
	 uint8_t  u8[48];
    uint32_t u32[16];
    uint64_t u64[8];
    xo256    xo;
    pcg64_t pcg;
    #if HAVE128
    cwg128_64_t cwg;
    #endif
  } state;
  rng_engine engine;
  int buf_word;
  int buf_byte;
  char *last_error;
  engine_fill fill;
  bool usefullmantissa;
  union {
    uint8_t u8[8*BUFSIZE];
    uint16_t u16[4*BUFSIZE];
    uint32_t u32[2*BUFSIZE];
    uint64_t u64[BUFSIZE];
  } buf;
};

typedef struct {
  char *name;
  char *description;
  rng_engine engine;
  int state_words;
  engine_fill fill;
} rng_entry;

#include "engines.inc"
#include "buffer_draw.inc"
#include "randutil.inc"
#include "distributions.inc"

static rng_entry rng_table[] = {
  {"x256++simd","xorshift256++, with SIMD accelaration (4x64)",FAST,    4, fill_x256ppsimd},
  {"x256++",   "xoshiro256++, Vigna & Blackman, 2019 (4x64)",  X256PP,  4, fill_x256pp    },
  {"x256**",   "xoshiro256**, Vigna & Blackman, 2019 (4x64)",  X256SS,  4, fill_x256ss    },
  {"xoro++",   "xoroshiro128++, Vigna & Blackman, 2016 (2x64)",XORO,    2, fill_xoro128pp },
  {"x128+",    "xorshift128+, Vigna, 2014 (2x64)",             X128P,   2, fill_x128p     },
  {"pcg64",    "PCG64-DXSM, O'Neill, 2014 (4x64)",             PCG64,   4, fill_pcg64     },
  {"sfc64",    "sfc64, Chris Doty-Humphrey, 2013 (4x64)",      SFC64,   4, fill_sfc64     },
  {"cwg128",   "cwg128-64, Działa, 2022 (5x64)",               CWG128,  5, fill_cwg128    },
  {"philox",   "Philox-4x64, Salmon & Moraes, 2011 (6x64)",    PHILOX,  6, fill_philox    },
  {"squares",  "squares64, Widynski, 2021 (2x64)",             SQUARES, 2, fill_squares   },
  {"chacha20", "ChaCha20, Bernstein, 2008 (6x64)",             CHACHA20,6, fill_chacha    },
  {"system",   "Operating system entropy source",              SYS,     0, fill_csprng    },
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
    rng->engine = FAST; // default engine
    rng->fill   = fill_x256ppsimd;
    return true;
  }
  char t[64];
  STRSET(t, s);
  for (int i = 0; t[i]; i++) {
    t[i] = TOLOWER(t[i]);
    if (t[i] == '-') t[i] = '_';
  }
  for (int i = 0; i < LEN(rng_table); i++) {
    if (!strcmp(t, rng_table[i].name)) {
      rng->engine = rng_table[i].engine;
      rng->fill   = rng_table[i].fill;
      return true;
    }
  }
  return false;  // unknown engine
}

randompack_rng *randompack_create(const char *engine) {
  randompack_rng *rng;
  // Create engine
  if (!ALLOC(rng, 1)) return 0;
  rng->engine = INVALID;
  if (!select_engine(engine, rng)) {
    rng->last_error = "unknown engine name (spelling error in requested engine)";
    return rng;
  }
  rng_entry *ent = find_entry(rng->engine);
  (void)ent;
  rand_init_randomize(rng);
  return rng;
}

bool randompack_seed(int seed, uint32_t *spawn_key, int nkey, randompack_rng *rng)  {
  if (!rng) return false;
  if (rng->engine == INVALID) {
    rng->last_error = "randompack seed: invalid rng";
    return false;
  }
 uint32_t seed32 = seed;
  // Use Melissa O'Neill's seed sequence
  if (nkey < 0 || (nkey > 0 && !spawn_key)) {
    rng->last_error = "randompack seed: invalid spawn_key arguments";
    return false;
  }
  uint32_t w[16];
  bool ok = seed_seq_seed(w, 16, seed32, spawn_key, nkey);
  if (!ok) {
    rng->last_error = "randompack seed: allocation failed";
    return false;
  }
  rng_entry *ent = find_entry(rng->engine);
  copy32(rng->state.u32, w, ent->state_words*2);
  if (rng->engine == FAST) {
    uint64_t tmp[4];
    copy64(tmp, rng->state.u64, 4);
    rng->state.xo.s0[0] = tmp[0];
    rng->state.xo.s1[0] = tmp[1];
    rng->state.xo.s2[0] = tmp[2];
    rng->state.xo.s3[0] = tmp[3];
  }
  rand_init(rng);
  return true;
}

bool randompack_randomize(randompack_rng *rng) {
  if (!rng) return false;
  if (rng->engine == INVALID) {
    rng->last_error = "randompack randomize: invalid rng";
    return false;
  }
  rand_init_randomize(rng);
  return true;
}

bool randompack_full_mantissa(randompack_rng *rng, bool enable) {
  if (!rng) return false;
  if (rng->engine == INVALID) {
    rng->last_error = "randompack full_mantissa: invalid rng";
    return false;
  }
  rng->last_error = 0;
  rng->usefullmantissa = enable;
  return true;
}

void randompack_free(randompack_rng *rng) {
  if (!rng) return;
  FREE(rng);
}

randompack_rng *randompack_duplicate(randompack_rng *src) {
  if (!src) return 0;
  randompack_rng *dst;
  if (!ALLOC(dst, 1)) return 0;
  memcpy(dst, src, sizeof(*dst));
  dst->last_error = 0;
  return dst;
}

enum { RNG_STATE_WORDS = sizeof(((randompack_rng *)0)->state)/sizeof(uint64_t) };

typedef struct {
  uint32_t version;
  uint32_t engine;
  uint64_t state_u64[RNG_STATE_WORDS];
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

//===================================== Setup ============================================

bool randompack_engines(char *engines, char *descriptions, int *nengines,
  int *eng_maxlen, int *desc_maxlen) {
  if (!nengines || !eng_maxlen || !desc_maxlen) return false;
  int n = LEN(rng_table);
  int emax = 1;
  int dmax = 1;
  for (int i = 0; i < n; i++) {
    int elen = (int)strlen(rng_table[i].name) + 1;
    int dlen = (int)strlen(rng_table[i].description) + 1;
    emax = max(emax, elen);
    dmax = max(dmax, dlen);
  }
  *nengines = n;
  *eng_maxlen = emax;
  *desc_maxlen = dmax;
  if (!engines) return true;
  if (!descriptions) return false;
  for (int i = 0; i < n; i++) {
    STRSETN(engines + i*emax, emax, rng_table[i].name);
    STRSETN(descriptions + i*dmax, dmax, rng_table[i].description);
  }
  return true;
}

char *randompack_last_error(randompack_rng *rng) {
  if (!rng) return 0;
  return rng->last_error;
}

//================================ Serialization =========================================

#include "serializations.inc"

bool randompack_serialize(uint8_t *buf, int *len, randompack_rng *rng) {
  // Returns the complete internal state of rng as an opaque byte buffer
  if (!rng) return false;
  rng->last_error = 0;
  if (rng->engine == SYS) {
    rng->last_error = "randompack serialize: system-csprng not supported";
    return false;
  }
  if (!len) {
    rng->last_error = "randompack serialize: len is null";
    return false;
  }
  int need = STATE_NEED;
  if (!buf) { // Report needed buffer size
    *len = need;
    return true;
  }
  if (*len < need) {
    rng->last_error = "randompack serialize: buffer too small";
    return false;
  }
  serialize(buf, *len, rng);
  return true;
}

bool randompack_deserialize(const uint8_t *buf, int len, randompack_rng *rng) {
  // Restores the rng state using a buffer obtained with randompack_serialize
  if (!rng) return false;
  rng->last_error = 0;
  if (rng->engine == SYS) {
    rng->last_error = "randompack deserialize: not supported for system rng";
    return false;
  }
  if (!buf || len <= 0) {
    rng->last_error = "randompack deserialize: invalid arguments";
    return false;
  }
  rng_blob blob = {0};
  memcpy(&blob, buf, min(len, STATE_NEED));
  rng_entry *ent = find_entry(blob.engine);
  int need = STATE_NEED;
  if (blob.version != 1 || !ent || len < need) {
    rng->last_error = "randompack deserialize: corrupt state buffer";
    return false;
  }
  if (blob.engine == SYS) {
    rng->last_error = "randompack deserialize: system-csprng not supported";
    return false;
  }
  if (rng->engine != INVALID && rng->engine != blob.engine) {
    rng->last_error = "randompack deserialize: engine mismatch";
    return false;
  }
  bool ok = deserialize(&blob, ent, rng);
  if (!ok) {
    rng->last_error = "randompack deserialize: allocation failed";
    return false;
  }
  return true;
}

//================================ State control =========================================

bool randompack_set_state(uint64_t state[], int nstate, randompack_rng *rng) {
  if (!rng) return false;
  rng->last_error = 0;
  if (!state || nstate < 0) {
    rng->last_error = "randompack set_state: invalid arguments";
	 return false;
  }
  rng_entry *ent = find_entry(rng->engine);
  if (rng->engine == SYS)
    rng->last_error = "randompack set_state: not supported for system-csprng";
  else if (nstate != ent->state_words)
    rng->last_error = "randompack set_state: wrong nstate for this engine";
  else if (rng->engine == X256PP || rng->engine == X256SS || rng->engine == FAST ||
           rng->engine == XORO || rng->engine == X128P) {
    bool all_zero = true;
    for (int i = 0; i < nstate; i++) {
      if (state[i] != 0) {
        all_zero = false;
        break;
      }
    }
    if (all_zero)
      rng->last_error = "randompack set_state: all-zero state is invalid";
  }
  if (rng->last_error) return false;
  set_state(state, nstate, rng);
  rand_init(rng);
  return true;
}

bool randompack_pcg64_set_state(uint64_t state, uint64_t inc, randompack_rng *rng) {
  if (!rng) return false;
  rng->last_error = 0;
  if (rng->engine != PCG64) {
    rng->last_error = "randompack pcg64_set_state: engine is not pcg64";
    return false;
  }
  pcg64_set_state(state, inc, rng);
  rand_init(rng);
  return true;
}

bool randompack_philox_set_state(randompack_philox_ctr ctr, randompack_philox_key key,
  randompack_rng *rng) {
  if (!rng) return false;
  rng->last_error = 0;
  if (rng->engine != PHILOX) {
    rng->last_error = "randompack philox_set_state: engine is not philox";
    return false;
  }
  philox_set_state(ctr, key, rng);
  rand_init(rng);
  return true;
}

bool randompack_squares_set_state(uint64_t ctr, uint64_t key,
  randompack_rng *rng) {
  if (!rng) return false;
  rng->last_error = 0;
  if (rng->engine != SQUARES) {
    rng->last_error = "randompack squares_set_state: engine is not squares64";
    return false;
  }
  squares_set_state(ctr, key, rng);
  rand_init(rng);
  return true;
}

//============================= Raw bitstreams ===========================================

bool randompack_raw(void *out, size_t nbytes, randompack_rng *rng) {
  if (!rng) return false;
  if (!out)
    rng->last_error = "invalid arguments to randompack_raw";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  draw_raw_copy((uint8_t*)out, nbytes, rng);
  return true;
}

bool randompack_uint8(uint8_t x[], size_t len, uint8_t bound, randompack_rng *rng) {
  if (!rng) return false;
  if (!x)
    rng->last_error = "randompack uint8: invalid arguments";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;  
  if (bound == 0)
    draw_raw_copy((uint8_t*)x, len, rng);
  else
	 rand_uint8(x, len, bound, rng);
  return true;
}

bool randompack_uint16(uint16_t x[], size_t len, uint16_t bound, randompack_rng *rng) {
  if (!rng) return false;
  if (!x)
    rng->last_error = "randompack uint16: invalid arguments";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;  
  if (bound == 0) {
    align16(rng);
    draw_raw_copy((uint8_t*)x, 2*len, rng);
  }
  else
	 rand_uint16(x, len, bound, rng);
  return true;
}

bool randompack_uint32(uint32_t x[], size_t len, uint32_t bound, randompack_rng *rng) {
  if (!rng) return false;
  if (!x)
    rng->last_error = "randompack uint32: invalid arguments";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;  
  if (bound == 0) {
    align32(rng);
    draw_raw_copy((uint8_t*)x, 4*len, rng);
  }
  else
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
  if (bound == 0) {
    align64(rng);
    draw_raw_copy((uint8_t*)x, 8*len, rng);
  }
  else
	 rand_uint64(x, len, bound, rng);
  return true;
}

//============================= Discrete distributions ===================================

bool randompack_int(int x[], size_t len, int m, int n, randompack_rng *rng) {
  if (!rng) return false;
  if (!x)
    rng->last_error = "invalid arguments to randompack_int";
  else if (m > n)
    rng->last_error = "randompack int: m must be <= n";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  uint32_t bound = (uint32_t)(n - m) + 1u;
  if (bound == 0) {
    align32(rng);
    draw_raw_copy((uint8_t*)x, 4*len, rng);
  }
  else
	 rand_int(x, len, m, bound, rng);
  return true;
}

bool randompack_long_long(long long x[], size_t len, long long m, long long n,
                          randompack_rng *rng) {
  if (!rng) return false;
  if (!x)
    rng->last_error = "invalid arguments to randompack_long_long";
  else if (m > n)
    rng->last_error = "randompack long long: m must be <= n";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  uint64_t bound = (uint64_t)(n - m) + 1ull;
  if (bound == 0) {
    align64(rng);
    draw_raw_copy((uint8_t*)x, 8*len, rng);
  }
  else
	 rand_long_long(x, len, m, bound, rng);
  return true;
}

// bool randompack_long_long(long long x[], size_t len, long long m, long long n,
//   randompack_rng *rng) {
//   if (!rng) return false;
//   if (!x)
//     rng->last_error = "invalid arguments to randompack_long_long";
//   else if (m > n)
//     rng->last_error = "randompack long long: m must be <= n";
//   else
//     rng->last_error = 0;
//   if (rng->last_error) return false;
//   if (m == LLONG_MIN && n == LLONG_MAX) {
//     rand_uint64((uint64_t*)x, len, 0, rng);
//     return true;
//   }
//   uint64_t span = (uint64_t)n - (uint64_t)m;
//   rand_uint64((uint64_t*)x, len, span + 1, rng);
//   for (size_t i = 0; i < len; i++) {
//     uint64_t u = (uint64_t)x[i] + (uint64_t)m;
//     x[i] = (long long)u;
//   }
//   return true;
// }

bool randompack_perm(int x[], int len, randompack_rng *rng) {
  if (!rng) return false;
  if (!x)
    rng->last_error = "invalid arguments to randompack_perm";
  else if (len > INT_MAX - 1)
    rng->last_error = "randompack perm: len must be <= 2^31 - 2";
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
    rng->last_error = "randompack sample: len must be <= 2^31 - 2";	 
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  rand_sample(x, len, k, rng);
  return true;
}

//========================== Continuous distributions ====================================

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

bool randompack_unif(double x[], size_t len, double a, double b,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || a >= b) {
    rng->last_error = "invalid arguments to randompack_unif";
    return false;
  }
  rng->last_error = 0;
  rand_dble(x, len, rng); // x in [0,1)
  if (a==0 && b==1) return true;
#if defined(FP_FAST_FMA)
  double w = nextafter(b - a, 0.0);
  for (size_t i = 0; i < len; i++) x[i] = fma(w, x[i], a);
#else
  double w = b - a;
  for (size_t i = 0; i < len; i++) {
    double y = a + w*x[i];
    y = y > b ? b : y;
    x[i] = y;
  }
#endif
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
  return true;
}

bool randompack_normal(double x[], size_t len, double mu, double sigma,
  randompack_rng *rng) { // general normal
  if (!rng) return false;
  if (!x || sigma <= 0) {
    rng->last_error = "invalid arguments to randompack_normal";
    return false;
  }
  rng->last_error = 0;
  rand_norm(x, len, rng);
  if (mu != 0 || sigma != 1)
    for (size_t i = 0; i < len; i++) x[i] = mu + sigma*x[i];
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

bool randompack_lognormal(double x[], size_t len, double mu, double sigma,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || sigma <= 0) {
    rng->last_error = "invalid arguments to randompack_lognormal";
    return false;
  }
  rng->last_error = 0;
  rand_lognormal(x, len, mu, sigma, rng);
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

bool randompack_chi2(double x[], size_t len, double nu, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || nu <= 0) {
    rng->last_error = "invalid arguments to randompack_chi2";
    return false;
  }
  rng->last_error = 0;
  fill_chi2(x, len, nu, rng);
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

bool randompack_gumbel(double x[], size_t len, double mu, double beta,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || beta <= 0) {
    rng->last_error = "invalid arguments to randompack_gumbel";
    return false;
  }
  rng->last_error = 0;
  fill_gumbel(x, len, mu, beta, rng);
  return true;
}

bool randompack_pareto(double x[], size_t len, double xm, double alpha,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || xm <= 0 || alpha <= 0) {
    rng->last_error = "invalid arguments to randompack_pareto";
    return false;
  }
  rng->last_error = 0;
  fill_pareto(x, len, xm, alpha, rng);
  return true;
}

bool randompack_weibull(double x[], size_t len, double shape, double scale,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || shape <= 0 || scale <= 0) {
    rng->last_error = "invalid arguments to randompack_weibull";
    return false;
  }
  rng->last_error = 0;
  fill_weibull(x, len, shape, scale, rng);
  return true;
}

bool randompack_skew_normal(double x[], size_t len, double mu, double sigma,
  double alpha, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || sigma <= 0) {
    rng->last_error = "invalid arguments to randompack_skew_normal";
    return false;
  }
  rng->last_error = 0;
  fill_skew_normal(x, len, mu, sigma, alpha, rng);
  return true;
}

//========================================================================================

//=========================== Multivariate normal ========================================

bool randompack_mvn(char *transp, double mu[], double Sig[], int d, size_t n, double X[],
                    int ldX, double L[], randompack_rng *rng) {
  if (!rng) return false;
  if (!Sig && !L)
    rng->last_error = "randompack mvn: either Sig or L must be specified";
  else if (!transp || (transp[0] != 'N' && transp[0] != 'T'))
    rng->last_error = "randompack mvn: transp must begin with 'N' or 'T'";
  else if ((!X && n > 0) || d <= 0 || (ldX <= 0 && X))
    rng->last_error = "randompack mvn: invalid arguments";
  else if (X && n > 0 && (size_t)ldX <
           ((transp[0] == 'N') ? n : (size_t)d))
    rng->last_error = "randompack mvn: invalid arguments";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;

  bool ok = rand_mvn(transp[0], mu, Sig, d, n, X, ldX, L, rng);
  if (!ok) {
    rng->last_error = "randompack mvn: memory allocation failed";
    return false;
  }
  else return true;
}

// =========== Include file with single precision (float) random generators ==============

// #include "randompack_float.inc"
