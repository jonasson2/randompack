// -*- C -*-
/* This is the jump function for the xoshiro256++ generator.
   It is equivalent to 2^128 calls to next(); it can be used to generate
   2^128 non-overlapping subsequences for parallel computations. */

static inline uint64_t xoshiro256pp_next(uint64_t s[4]) {
  uint64_t result = ROTL(s[0] + s[3], 23) + s[0];
  uint64_t t = s[1] << 17;
  s[2] ^= s[0];
  s[3] ^= s[1];
  s[1] ^= s[2];
  s[0] ^= s[3];
  s[2] ^= t;
  s[3] = ROTL(s[3], 45);
  return result;
}

static inline void xoshiro256pp_jump(uint64_t s[4]) {
  static const uint64_t JUMP[] = {
    0x180ec6d33cfd0abaULL,
    0xd5a61266f0c9392cULL,
    0xa9582618e03fc9aaULL,
    0x39abdc4529b1661cULL
  };
  uint64_t s0 = 0;
  uint64_t s1 = 0;
  uint64_t s2 = 0;
  uint64_t s3 = 0;
  for (int i = 0; i < 4; i++) {
    for (int b = 0; b < 64; b++) {
      if (JUMP[i] & (UINT64_C(1) << b)) {
        s0 ^= s[0];
        s1 ^= s[1];
        s2 ^= s[2];
        s3 ^= s[3];
      }
      xoshiro256pp_next(s);  // your next() / step function
    }
  }
  s[0] = s0;
  s[1] = s1;
  s[2] = s2;
  s[3] = s3;
}
