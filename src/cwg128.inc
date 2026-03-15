// -*- C -*-
static inline void fill_cwg128(uint64_t *buf, size_t len, randompack_state *state) {
  uint64_t *out = buf;
  // State layout assumed:
  //   state->cwg.x    : __uint128_t
  //   state->cwg.a    : uint64_t
  //   state->cwg.weyl : uint64_t
  //   state->cwg.s    : uint64_t (must be odd)
  __uint128_t c0 = state->cwg128.c[0]; // odd increment
  __uint128_t c1 = state->cwg128.c[1];
  __uint128_t c2 = state->cwg128.c[2];
  __uint128_t c3 = state->cwg128.c[3];
  for (size_t i=0; i<len; i += 2) {
    c1 = (c1 >> 1) * ((c2 += c1) | 1) ^ (c3 += c0);
    __uint128_t z = (c2 >> 96) ^ c1;
    out[i] = (uint64_t)z;
    out[i + 1] = (uint64_t)(z >> 64);
  }
  state->cwg128.c[0] = c0;
  state->cwg128.c[1] = c1;
  state->cwg128.c[2] = c2;
  state->cwg128.c[3] = c3;

  // __uint128_t x = state->cwg.x;
  // uint64_t a = state->cwg.a;
  // uint64_t weyl = state->cwg.weyl;
  // uint64_t s = state->cwg.s;
  // for (size_t i = 0; i < len; i++) {
  //   a += (uint64_t)x; // modulo 2^64
  //   weyl += s;
  //   // x = (x | 1) * ((a += x) >> 1) ^ (weyl += s);
  //   x = (x | 1) * ((__uint128_t)(a >> 1)) ^ (__uint128_t)weyl;
  //   // return a >> 48 ^ x;  (take low 64 bits of x for a 64-bit output)
  //   out[i] = (a >> 48) ^ (uint64_t)x;
  // }
  // state->cwg.x = x;
  // state->cwg.a = a;
  // state->cwg.weyl = weyl;
  // state->cwg.s = s;
}
