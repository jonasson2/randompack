// -*- C -*-
typedef struct {
  uint64_t lo;
  uint64_t hi;
} rp_u128;

#define U128_SET(x, lo_, hi_) do { \
  (x).lo = (lo_);                  \
  (x).hi = (hi_);                  \
} while (0)

#define U128_LO(x) ((x).lo)

#define U128_OR1(x) do { \
  (x).lo |= 1;           \
} while (0)

#define U128_XOR64(x, v) do { \
  (x).lo ^= (v);              \
} while (0)

#define U128_MUL64(out, x, b) do {          \
  uint64_t _hi0, _lo0, _hi1, _lo1;           \
  MUL64_WIDE((x).lo, (b), _hi0, _lo0);       \
  MUL64_WIDE((x).hi, (b), _hi1, _lo1);       \
  (out).lo = _lo0;                           \
  (out).hi = _hi0 + _lo1;                    \
} while (0)

static inline void fill_cwg128(uint64_t *buf, size_t len, randompack_state *state) {
  uint64_t *out = buf;
  // State layout assumed:
  //   state->u64[0:1]: x low, x high
  //   state->u64[2]  : a
  //   state->u64[3]  : weyl
  //   state->u64[4]  : s (must be odd)
  rp_u128 x;
  U128_SET(x, state->u64[0], state->u64[1]);
  uint64_t a = state->u64[2];
  uint64_t weyl = state->u64[3];
  uint64_t s = state->u64[4];
  for (size_t i = 0; i < len; i++) {
    a += U128_LO(x);
    weyl += s;
    // x = (x | 1) * ((a += x) >> 1) ^ (weyl += s);
    U128_OR1(x);
    rp_u128 t;
    U128_MUL64(t, x, a >> 1);
    x = t;
    U128_XOR64(x, weyl);
    // return a >> 48 ^ x;  (take low 64 bits of x for a 64-bit output)
    out[i] = (a >> 48) ^ U128_LO(x);
  }
  state->u64[0] = x.lo;
  state->u64[1] = x.hi;
  state->u64[2] = a;
  state->u64[3] = weyl;
  state->u64[4] = s;
}
