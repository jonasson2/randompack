// -*- C -*-
#include "chacha-portable.c"

static inline void fill_chacha(randompack_rng *rng) {
  uint8_t *dst = (uint8_t *)rng->buf.u64;
  uint8_t zeros[BUFSIZE*8] = {0};
  int nbytes = BUFSIZE*8;
  uint32_t counter = rng->state.u32[11];
  uint8_t *key     = &rng->state.u8[0];
  uint8_t *nonce   = &rng->state.u8[32];
  chacha20_xor_stream(dst, zeros, nbytes, key, nonce, counter);
  rng->state.u32[11] += BUFSIZE/8; // count of 64 byte blocks
}
