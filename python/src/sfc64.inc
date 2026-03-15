// -*- C -*-
// static inline void fill_sfc64(uint64_t *buf, size_t len, randompack_state *state) {
//   uint64_t *out = buf;
//   uint64_t a = state->u64[0];
//   uint64_t b = state->u64[1];
//   uint64_t c = state->u64[2];
//   uint64_t counter = state->u64[3];
//   for (size_t i = 0; i < len; i++) {
//     uint64_t tmp = a + b + counter++;
//     a = b ^ (b >> 11);
//     b = c + (c << 3);
//     c = (c << 24) | (c >> 40);
//     c += tmp;
//     out[i] = tmp;
//   }
//   state->u64[0] = a;
//   state->u64[1] = b;
//   state->u64[2] = c;
//   state->u64[3] = counter;
// }


static inline void fill_sfc64(uint64_t *buf, size_t len, randompack_state *state) {
  uint64_t *out = buf;
  uint64_t a = state->u64[0];
  uint64_t b = state->u64[1];
  uint64_t c = state->u64[2];
  uint64_t counter = state->u64[3];
  for (size_t i=0; i<len; i += 4) {
    uint64_t tmp0 = a + b + counter++;
    uint64_t a0 = b ^ (b >> 11);
    uint64_t b0 = c + (c << 3);
    uint64_t c0 = (c << 24) | (c >> 40);
    c0 += tmp0;
    uint64_t tmp1 = a0 + b0 + counter++;
    uint64_t a1 = b0 ^ (b0 >> 11);
    uint64_t b1 = c0 + (c0 << 3);
    uint64_t c1 = (c0 << 24) | (c0 >> 40);
    c1 += tmp1;
    uint64_t tmp2 = a1 + b1 + counter++;
    uint64_t a2 = b1 ^ (b1 >> 11);
    uint64_t b2 = c1 + (c1 << 3);
    uint64_t c2 = (c1 << 24) | (c1 >> 40);
    c2 += tmp2;
    uint64_t tmp3 = a2 + b2 + counter++;
    a = b2 ^ (b2 >> 11);
    b = c2 + (c2 << 3);
    c = (c2 << 24) | (c2 >> 40);
    c += tmp3;
    out[i] = tmp0;
    out[i+1] = tmp1;
    out[i+2] = tmp2;
    out[i+3] = tmp3;
  }
  state->u64[0] = a;
  state->u64[1] = b;
  state->u64[2] = c;
  state->u64[3] = counter;
}
