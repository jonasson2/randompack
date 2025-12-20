// -*- mode: c -*-
static inline void fill_x128p(randompack_rng *rng) {
  // With parameters (23,18,5), from Vigna (2014, arXiv:1404.0390)
  uint64_t *out = rng->buf;
  size_t n = BUFSIZE;
  uint64_t s0 = rng->state.u64[0];
  uint64_t s1 = rng->state.u64[1];
  for (size_t i = 0; i < n; i++) {
    uint64_t t = s1 ^ (s1 << 23);
    uint64_t result = s0 + t;
    s1 = s0;
    s0 = t ^ s0 ^ (t >> 18) ^ (s0 >> 5);
    out[i] = result;
  }
  rng->state.u64[0] = s0;
  rng->state.u64[1] = s1;
}
