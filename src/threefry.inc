// -*- C -*-
// This file is extracted from the Random123 official github repository as follows:
//
// 1. Clone https://github.com/DEShawResearch/Random123.git
// 2. cd to Random123/include/Random123, and run the C preprocessor:
//      clang -E -P -DR123_NO_STDINC threefry.h | clang-format > threefry.inc
// 3. Edit threefry.inc by hand to retain only threefry4x64_R and what it depends on
// 4. Rename the function to threefry4x64 and hardwire R = 10
//      (cf. define on line 366 in threefry.h)
// 5. Change __inline__ to inline and remove __atribute__((always_inline)
//
// The Random123 commit used is 1a2b3c4d5e (February 2025). This version was
// tested to give identical threefry4x64 outputs as the official library built
// with ‘make’ in Random123/examples. BSD license, see License-Random123.txt.
//
// Kristján Jónasson, jonasson@hi.is, 2025

typedef long long int64_t;

typedef unsigned long long uint64_t;

struct r123array4x64 {
  uint64_t v[4];
};
enum r123_enum_threefry64x4 {
  R_64x4_0_0 = 14,
  R_64x4_0_1 = 16,
  R_64x4_1_0 = 52,
  R_64x4_1_1 = 57,
  R_64x4_2_0 = 23,
  R_64x4_2_1 = 40,
  R_64x4_3_0 = 5,
  R_64x4_3_1 = 37,
  R_64x4_4_0 = 25,
  R_64x4_4_1 = 33,
  R_64x4_5_0 = 46,
  R_64x4_5_1 = 12,
  R_64x4_6_0 = 58,
  R_64x4_6_1 = 22,
  R_64x4_7_0 = 32,
  R_64x4_7_1 = 32
};

static inline uint64_t RotL_64(uint64_t x, unsigned int N);

static inline uint64_t RotL_64(uint64_t x, unsigned int N) {
  return (x << (N & 63)) | (x >> ((64 - N) & 63));
}
typedef struct r123array4x64 threefry4x64_ctr_t;
typedef struct r123array4x64 threefry4x64_key_t;

static inline threefry4x64_ctr_t threefry4x64(threefry4x64_ctr_t in,
                                              threefry4x64_key_t k) {
  uint64_t X0, X1, X2, X3;
  uint64_t ks0, ks1, ks2, ks3, ks4;
  ks4 = ((0xA9FC1A22) + (((uint64_t)(0x1BD11BDA)) << 32));
  ks0 = k.v[0];
  X0 = in.v[0] + ks0;
  ks4 ^= ks0;
  ks1 = k.v[1];
  X1 = in.v[1] + ks1;
  ks4 ^= ks1;
  ks2 = k.v[2];
  X2 = in.v[2] + ks2;
  ks4 ^= ks2;
  ks3 = k.v[3];
  X3 = in.v[3] + ks3;
  ks4 ^= ks3;

  X0 += X1;
  X1 = RotL_64(X1, R_64x4_0_0);
  X1 ^= X0;
  X2 += X3;
  X3 = RotL_64(X3, R_64x4_0_1);
  X3 ^= X2;

  X0 += X3;
  X3 = RotL_64(X3, R_64x4_1_0);
  X3 ^= X0;
  X2 += X1;
  X1 = RotL_64(X1, R_64x4_1_1);
  X1 ^= X2;

  X0 += X1;
  X1 = RotL_64(X1, R_64x4_2_0);
  X1 ^= X0;
  X2 += X3;
  X3 = RotL_64(X3, R_64x4_2_1);
  X3 ^= X2;

  X0 += X3;
  X3 = RotL_64(X3, R_64x4_3_0);
  X3 ^= X0;
  X2 += X1;
  X1 = RotL_64(X1, R_64x4_3_1);
  X1 ^= X2;

  X0 += ks1;
  X1 += ks2;
  X2 += ks3;
  X3 += ks4;
  X3 += 1;

  X0 += X1;
  X1 = RotL_64(X1, R_64x4_4_0);
  X1 ^= X0;
  X2 += X3;
  X3 = RotL_64(X3, R_64x4_4_1);
  X3 ^= X2;

  X0 += X3;
  X3 = RotL_64(X3, R_64x4_5_0);
  X3 ^= X0;
  X2 += X1;
  X1 = RotL_64(X1, R_64x4_5_1);
  X1 ^= X2;

  X0 += X1;
  X1 = RotL_64(X1, R_64x4_6_0);
  X1 ^= X0;
  X2 += X3;
  X3 = RotL_64(X3, R_64x4_6_1);
  X3 ^= X2;

  X0 += X3;
  X3 = RotL_64(X3, R_64x4_7_0);
  X3 ^= X0;
  X2 += X1;
  X1 = RotL_64(X1, R_64x4_7_1);
  X1 ^= X2;

  X0 += ks2;
  X1 += ks3;
  X2 += ks4;
  X3 += ks0;
  X3 += 2;

  X0 += X1;
  X1 = RotL_64(X1, R_64x4_0_0);
  X1 ^= X0;
  X2 += X3;
  X3 = RotL_64(X3, R_64x4_0_1);
  X3 ^= X2;

  X0 += X3;
  X3 = RotL_64(X3, R_64x4_1_0);
  X3 ^= X0;
  X2 += X1;
  X1 = RotL_64(X1, R_64x4_1_1);
  X1 ^= X2;

  X0 += X1;
  X1 = RotL_64(X1, R_64x4_2_0);
  X1 ^= X0;
  X2 += X3;
  X3 = RotL_64(X3, R_64x4_2_1);
  X3 ^= X2;

  X0 += X3;
  X3 = RotL_64(X3, R_64x4_3_0);
  X3 ^= X0;
  X2 += X1;
  X1 = RotL_64(X1, R_64x4_3_1);
  X1 ^= X2;

  X0 += ks3;
  X1 += ks4;
  X2 += ks0;
  X3 += ks1;
  X3 += 3;

  X0 += X1;
  X1 = RotL_64(X1, R_64x4_4_0);
  X1 ^= X0;
  X2 += X3;
  X3 = RotL_64(X3, R_64x4_4_1);
  X3 ^= X2;

  X0 += X3;
  X3 = RotL_64(X3, R_64x4_5_0);
  X3 ^= X0;
  X2 += X1;
  X1 = RotL_64(X1, R_64x4_5_1);
  X1 ^= X2;

  X0 += X1;
  X1 = RotL_64(X1, R_64x4_6_0);
  X1 ^= X0;
  X2 += X3;
  X3 = RotL_64(X3, R_64x4_6_1);
  X3 ^= X2;

  X0 += X3;
  X3 = RotL_64(X3, R_64x4_7_0);
  X3 ^= X0;
  X2 += X1;
  X1 = RotL_64(X1, R_64x4_7_1);
  X1 ^= X2;

  X0 += ks4;
  X1 += ks0;
  X2 += ks1;
  X3 += ks2;
  X3 += 4;

  X0 += X1;
  X1 = RotL_64(X1, R_64x4_0_0);
  X1 ^= X0;
  X2 += X3;
  X3 = RotL_64(X3, R_64x4_0_1);
  X3 ^= X2;

  X0 += X3;
  X3 = RotL_64(X3, R_64x4_1_0);
  X3 ^= X0;
  X2 += X1;
  X1 = RotL_64(X1, R_64x4_1_1);
  X1 ^= X2;

  X0 += X1;
  X1 = RotL_64(X1, R_64x4_2_0);
  X1 ^= X0;
  X2 += X3;
  X3 = RotL_64(X3, R_64x4_2_1);
  X3 ^= X2;

  X0 += X3;
  X3 = RotL_64(X3, R_64x4_3_0);
  X3 ^= X0;
  X2 += X1;
  X1 = RotL_64(X1, R_64x4_3_1);
  X1 ^= X2;

  X0 += ks0;
  X1 += ks1;
  X2 += ks2;
  X3 += ks3;
  X3 += 5;

  threefry4x64_ctr_t ret = {{X0, X1, X2, X3}};
  return ret;
}
