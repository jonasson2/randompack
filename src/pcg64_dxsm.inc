// -*- mode: c -*-

// Minor changes in this file from the original pcg64_dxsm.c, downloaded from Tony Finch’s
// pcg-dxsm GitHub repository (https://github.com/fanf2/pcg-dxsm) on 30 Nov. 2025;
// pcg-dxsm implements Melissa O'Neill's PCG64-DXSM rng (see https://www.pcg-random.org):
//   Moved definitions of the increment constant and pcg_ulong_t from the Makefile to here
//   Now uses randompack_rng instead of pcg64_t
// Kristján Jónasson, jonasson@hi.is, Dept. of Computer Science, University of Iceland

// SPDX-License-Identifier: 0BSD OR MIT-0

/*
 * pcg64 dxsm by Melissa O'Neill
 */

#include <stdint.h>

#define PCG_INCREMENT  ((pcg_ulong_t)6364136223846793005ULL << 64 |     \
                        (pcg_ulong_t)1442695040888963407ULL)
#define pcg_ulong_t      __uint128_t

static inline uint64_t
pcg64_random_fast(randompack_rng *rng) {
  /* cheap (half-width) multiplier */
  const uint64_t mul = 15750249268501108917ULL;
  /* linear congruential generator */
  pcg_ulong_t state = rng->state.u128;
  rng->state.u128 = state*mul + PCG_INCREMENT;
  /* DXSM (double xor shift multiply) permuted output */
  uint64_t hi = (uint64_t)(state >> 64);
  uint64_t lo = (uint64_t)(state | 1);
  hi ^= hi >> 32;
  hi *= mul;
  hi ^= hi >> 48;
  hi *= lo;
  return (hi);
}
