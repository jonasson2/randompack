// -*- C -*-
#include "chacha-portable.c"
static inline void fill_chacha(randompack_rng *rng) {
  uint8_t *dst = (uint8_t *)rng->buf;
  int nbytes = BUFSIZE*8;
  memset(dst, 0, nbytes);
  uint32_t counter = rng->state.u32[11];
  uint8_t *key     = &rng->state.u8[0];
  uint8_t *nonce   = &rng->state.u8[32];
  chacha20_xor_stream(dst, dst, nbytes, key, nonce, counter);
  rng->state.u32[11] += BUFSIZE/8; // count of 64 byte blocks
}

// // Define CHACHA20_IMPLEMENTATION in exactly one translation unit before including.
// #define CHACHA20_IMPLEMENTATION
// #include "chacha20.h"
// #undef CHACHA20_IMPLEMENTATION

// uint64_t chachacha(randompack_rng *rng)
// {
//   ChaCha20_Ctx *ctx = (ChaCha20_Ctx *)rng->extra_state;
//   uint8_t buf[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//   // Optional sanity checks:
//   // assert(rng->engine == RNG_ENGINE_CHACHA20);
//   // assert(ctx != 0);

//   // Get 8 bytes of fresh keystream
//   ChaCha20_xor(ctx, buf, 8);

//   // Pack as little-endian into 64 bits
//   uint64_t x =
//       (uint64_t)buf[0]
//     | (uint64_t)buf[1] << 8
//     | (uint64_t)buf[2] << 16
//     | (uint64_t)buf[3] << 24
//     | (uint64_t)buf[4] << 32
//     | (uint64_t)buf[5] << 40
//     | (uint64_t)buf[6] << 48
//     | (uint64_t)buf[7] << 56;

//   return x;
// }
