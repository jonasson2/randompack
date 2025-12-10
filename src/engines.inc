// -*- mode: c; -*-
// RNG engine implementations
#include "xorshift128p.inc"
#include "xoshiro256pp.inc"
#include "xoshiro256ss.inc"
#include "chachacha.inc"
#include "counter_based.inc"
#include "csprng.inc"

#ifdef HAVE128
#include "pcg64_dxsm.inc"
#endif

#ifdef HAVE128MUL
#include "philox_rng.inc"
#endif

static inline uint32_t PM_rand_bits(randompack_rng *rng) {
  // returns an integer in [1, 2^31 - 2]
  const uint32_t a = 16807, m = mersenne8, q = m/a, r = m%a;
  uint32_t s1, s2;
  s1 = a*(rng->state.u32 % q);
  s2 = r*(rng->state.u32/q);
  if (s1 > s2) rng->state.u32 = s1 - s2; else rng->state.u32 = (m - s2) + s1;
  return rng->state.u32;
}
