// -*- mode: c -*-

// The core of this file is copied from pcg64_dxsm.c in Tony Finch’s pcg-dxsm GitHub
// repository (github.com/fanf2/pcg-dxsm, 79eea6ae8e83, 2025-10-29) which implements
// Melissa O'Neill's PCG64-DXSM rng (see https://www.pcg-random.org). The main
// computational lines are taken verbatim, as identified with comments.
//
// SPDX-License-Identifier: 0BSD OR MIT-0

static inline void
fill_pcg64_opt(randompack_rng *rng) {
  uint64_t *out = rng->buf;

  /* cheap (half-width) multiplier */
  const uint64_t mul = 15750249268501108917ULL;  // Identical to original

  /* linear congruential generator */
  pcg_ulong_t next = rng->state.pcg.state;  // Functionally same as original
  pcg_ulong_t inc = rng->state.pcg.inc;     //
  
  for (int i=0; i<BUFSIZE; i++) {  // Added fill-buffer-loop
    /* linear congruential generator */
	 pcg_ulong_t state = next;               // Functionally same as original
    next = state*mul + inc;                 //
    /* DXSM (double xor shift multiply) permuted output */
    uint64_t hi = (uint64_t)(state >> 64);  //
    uint64_t lo = (uint64_t)(state | 1);    // Identical to original
    hi ^= hi >> 32;                         //
    hi *= mul;                              //
    hi ^= hi >> 48;                         //
    hi *= lo;                               //
    out[i] = hi; // Original has return hi;
  }
  rng->state.pcg.state = next;
  rng->state.pcg.inc = inc;
}
