// -*- C -*-
static inline void fill_sfc64(uint64_t *buf, size_t len, randompack_state *state) {
  uint64_t *out = buf;
  uint64_t a = state->u64[0];
  uint64_t b = state->u64[1];
  uint64_t c = state->u64[2];
  uint64_t counter = state->u64[3];
  for (size_t i = 0; i < len; i++) {
    uint64_t tmp = a + b + counter++;
    a = b ^ (b >> 11);
    b = c + (c << 3);
    c = (c << 24) | (c >> 40);
    c += tmp;
    out[i] = tmp;
  }
  state->u64[0] = a;
  state->u64[1] = b;
  state->u64[2] = c;
  state->u64[3] = counter;
}
