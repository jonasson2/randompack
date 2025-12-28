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
