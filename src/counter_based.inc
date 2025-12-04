// -*- C -*-
// Counter-based randoms using Random123 threefry4x64 and philox4x64.

#include "random123_4x64.inc"

static inline void rand_3fry_64bits(uint64_t x[], int n, randompack_counter c,
                                    randompack_3fry_key k) {
  int filled = 0;
  while (filled < n) {
    randompack_counter out = threefry4x64(c, k);
    for (int j = 0; j < 4 && filled < n; j++)
      x[filled++] = out.v[j];
    c.v[0]++;
  }
}

static inline void rand_phil_64bits(uint64_t x[], int n, randompack_counter c,
                                    randompack_philox_key k) {
  int filled = 0;
  while (filled < n) {
    randompack_counter out = philox4x64(c, k);
    for (int j = 0; j < 4 && filled < n; j++)
      x[filled++] = out.v[j];
    c.v[0]++;
  }
}
