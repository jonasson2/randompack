// -*- C -*-

#ifndef RANDOMPACK_INTERNAL_H
#define RANDOMPACK_INTERNAL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "randompack_config.h"

#if HAVE128
#include "cwg128_64.h"
#include "pcg64.h"
#else
#include "pcg64_emul.h"
#endif

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
    uint8_t u8[48];
    uint32_t u32[16];
    uint64_t u64[8];
    xo256 xo;
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
  bool bitexact;
  bool cpu_has_avx2;
  union {
    uint8_t u8[8*BUFSIZE];
    uint16_t u16[4*BUFSIZE];
    uint32_t u32[2*BUFSIZE];
    uint64_t u64[BUFSIZE];
  } buf;
};

#if defined(BUILD_AVX2)
bool cpu_has_avx2(void);
void fill_fast_avx2(randompack_rng *rng, size_t len);
void rand_dble_avx2(double x[], size_t len, randompack_rng *rng);
void rand_float_avx2(float x[], size_t len, randompack_rng *rng);
#if defined(RANDOMPACK_TEST_HOOKS)
int randompack_avx2_used(void);
void randompack_avx2_reset(void);
#endif
#endif

#endif
