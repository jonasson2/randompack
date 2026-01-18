// -*- C -*-
static inline void fill_sfc64(randompack_rng *rng) {
  uint64_t *out = rng->buf.u64;
  uint64_t a = rng->state.u64[0];
  uint64_t b = rng->state.u64[1];
  uint64_t c = rng->state.u64[2];
  uint64_t counter = rng->state.u64[3];
  for (int i = 0; i < BUFSIZE; i++) {
    uint64_t tmp = a + b + counter++;
    a = b ^ (b >> 11);
    b = c + (c << 3);
    c = (c << 24) | (c >> 40);
    c += tmp;
    out[i] = tmp;
  }
  rng->state.u64[0] = a;
  rng->state.u64[1] = b;
  rng->state.u64[2] = c;
  rng->state.u64[3] = counter;
}
