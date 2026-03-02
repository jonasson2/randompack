// -*- C -*-
#include <stdint.h>

#define RANLUXPP_NO_TYPE 1
#define ranluxpp_t rp_ranluxpp_t
#define ranluxpp_init rp_ranluxpp_init
#define ranluxpp_nextstate rp_ranluxpp_nextstate
#define ranluxpp_jump rp_ranluxpp_jump
#define ranluxpp_fprintf rp_ranluxpp_fprintf
#include "ranluxpp-portable/ranluxpp.c"
#undef ranluxpp_fprintf
#undef ranluxpp_jump
#undef ranluxpp_nextstate
#undef ranluxpp_init
#undef ranluxpp_t
#undef RANLUXPP_NO_TYPE

ALWAYS_INLINE void ranluxpp_portable_seed(randompack_rng *rng, uint64_t seed) {
  rp_ranluxpp_t *r = &rng->state.rpp;
  rp_ranluxpp_init(r, seed, 2048);
}

ALWAYS_INLINE void ranluxpp_portable_set_state(randompack_rng *rng,
  const uint64_t *state) {
  rp_ranluxpp_t *r = &rng->state.rpp;
  rp_ranluxpp_init(r, 0, 2048);
  for (int i = 0; i < 9; i++) r->x[i] = state[i];
}

ALWAYS_INLINE void fill_ranluxpp_portable(randompack_rng *rng, size_t len) {
  rp_ranluxpp_t *r = &rng->state.rpp;
  uint64_t *out = rng->buf.u64;
  size_t nblocks = len/9;
  size_t rem = len - nblocks*9;
  for (size_t b = 0; b < nblocks; b++) {
    rp_ranluxpp_nextstate(r);
    memcpy(out + 9*b, r->x, 9*sizeof(uint64_t));
  }
  if (rem > 0) {
    rp_ranluxpp_nextstate(r);
    size_t i = nblocks*9;
    for (size_t j = 0; j < rem; j++) out[i + j] = r->x[j];
  }
}
