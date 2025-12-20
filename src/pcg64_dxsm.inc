// -*- mode: c -*-

// This file is copied verbatim from pcg64_dxsm.c in Tony Finch’s pcg-dxsm GitHub
// repository (github.com/fanf2/pcg-dxsm, 79eea6ae8e83, 2025-10-29) which implements
// Melissa O'Neill's PCG64-DXSM rng (see https://www.pcg-random.org).
//
// The only change is to comment out the double function at the bottom. See also pcg64.h

// SPDX-License-Identifier: 0BSD OR MIT-0

/*
 * pcg64 dxsm by Melissa O'Neill
 */

static inline uint64_t
pcg64_random_fast(pcg64_t *rng) {
	/* cheap (half-width) multiplier */
	const uint64_t mul = 15750249268501108917ULL;
	/* linear congruential generator */
	pcg_ulong_t state = rng->state;
	rng->state = state * mul + rng->inc;
	/* DXSM (double xor shift multiply) permuted output */
	uint64_t hi = (uint64_t)(state >> 64);
	uint64_t lo = (uint64_t)(state | 1);
	hi ^= hi >> 32;
	hi *= mul;
	hi ^= hi >> 48;
	hi *= lo;
	return (hi);
}

static inline void fill_pcg64_opt(randompack_rng *rng) {
  uint64_t *out = rng->buf;
  size_t n = BUFSIZE;
  pcg64_t st = rng->state.pcg;
  for (size_t i = 0; i < n; i++) out[i] = pcg64_random_fast(&st);
  rng->state.pcg = st;
}

// static inline double
// pcg64_double(pcg64_t *rng) {
// 	return ((double)(pcg64_random(rng) >> 11) * 0x1.0p-53);
// }
