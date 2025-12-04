// -*- C -*-

// This file is extracted from the Random123 official github repository as follows:
//
// 1. Clone https://github.com/DEShawResearch/Random123.git
// 2. cd to Random123/include/Random123, and run the C preprocessor:
//      clang -E -P -DR123_NO_STDINC philox.h | clang-format > philox.inc
// 3. Edit philox.inc by hand to retain only philox4x64_R and what it depends on
// 4. Rename the function to philox4x64 and hardwire R = 10
//      (cf. define on line 366 in philox.h)
// 5. Change __inline__ to inline
//
// The Random123 commit used is 1a2b3c4d5e (February 2025). This version was
// tested to give identical philox4x64 outputs as the official library built
// with ‘make’ in Random123/examples. BSD license, see License-Random123.txt.
//
// Kristján Jónasson, jonasson@hi.is, 2025
 
struct r123array2x64 {
  uint64_t v[2];
};

struct r123array4x64 {
  uint64_t v[4];
};

static inline uint64_t mulhilo64(uint64_t a, uint64_t b, uint64_t *hip) {
  __uint128_t product = ((__uint128_t)a) * ((__uint128_t)b);
  *hip = product >> 64;
  return (uint64_t)product;
}

static inline struct r123array2x64
_philox4x64bumpkey(struct r123array2x64 key) {
  key.v[0] += (0x9E3779B97F4A7C15ULL);
  key.v[1] += (0xBB67AE8584CAA73BULL);
  return key;
}

static inline struct r123array4x64
_philox4x64round(struct r123array4x64 ctr, struct r123array2x64 key) {
  uint64_t hi0;
  uint64_t hi1;
  uint64_t lo0 = mulhilo64((0xD2E7470EE14C6C93ULL), ctr.v[0], &hi0);
  uint64_t lo1 = mulhilo64((0xCA5A826395121157ULL), ctr.v[2], &hi1);
  struct r123array4x64 out = {
      {hi1 ^ ctr.v[1] ^ key.v[0], lo1, hi0 ^ ctr.v[3] ^ key.v[1], lo0}};
  return out;
}

typedef struct r123array4x64 philox4x64_ctr_t;
typedef struct r123array2x64 philox4x64_key_t;

static inline philox4x64_ctr_t
philox4x64(philox4x64_ctr_t ctr, philox4x64_key_t key) {
  ctr = _philox4x64round(ctr, key);
  key = _philox4x64bumpkey(key); ctr = _philox4x64round(ctr, key);
  key = _philox4x64bumpkey(key); ctr = _philox4x64round(ctr, key);
  key = _philox4x64bumpkey(key); ctr = _philox4x64round(ctr, key);
  key = _philox4x64bumpkey(key); ctr = _philox4x64round(ctr, key);
  key = _philox4x64bumpkey(key); ctr = _philox4x64round(ctr, key);
  key = _philox4x64bumpkey(key); ctr = _philox4x64round(ctr, key);
  key = _philox4x64bumpkey(key); ctr = _philox4x64round(ctr, key);
  key = _philox4x64bumpkey(key); ctr = _philox4x64round(ctr, key);
  key = _philox4x64bumpkey(key); ctr = _philox4x64round(ctr, key);
  return ctr;
}
