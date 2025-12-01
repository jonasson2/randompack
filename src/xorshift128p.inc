// -*- mode: c -*-
static inline uint64_t xorshift128p(randompack_rng *rng) {
  // With parameters (23,18,5), from Vigna (2014, arXiv:1404.0390)
  uint64_t s1 = rng->state.u64[0];
  uint64_t s0 = rng->state.u64[1];
  rng->state.u64[0] = s0;
  s1 ^= s1 << 23; // a
  uint64_t result = s0 + s1;
  rng->state.u64[1] = s1^s0^(s1 >> 18)^(s0 >> 5); // b, c
  return result;
}
