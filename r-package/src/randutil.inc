// -*- mode: c; -*-
#include <time.h>

static void fill_bytes(uint8_t *a, int len, uint64_t *x) {
  for (int k = 0; k < len; k += 8) {
    uint64_t mixed = rand_splitmix64(x);
    memcpy(a + k, &mixed, min(8, len - k));
  }
}

static void entropy_fill(randompack_rng *rng, void *buffer, size_t len) {
  bool ok = crypto_random(buffer, len);
  if (!ok) {  // fallback on esoteric systems
    uint64_t x = clock_nsec();
    x ^= (uintptr_t)rng;
    fill_bytes(buffer, len, &x);
  }
}

static inline void normalize_state(randompack_rng *rng) {
  // Make state nonzero for xor-family, odd for cwg128 & pcg64.
  switch (rng->engine) {
  case X256PP:
  case X256SS:
  case FAST:
  case XORO:
  case X128P:
    if (rng->state.u64[0] == 0) rng->state.u64[0] = 1;
    break;
  case CWG128:
    rng->state.u64[4] |= 1;
    break;
  case PCG64:
    rng->state.u64[2] |= 1;
    break;
  default:
    break;
  }
}

static inline void fill_stream_states_with_jump(randompack_rng *rng) {
  if (rng->engine == FAST) {
    xo256 *st = &rng->state.xo;
    for (int i = 1; i < 4; i++) {
      st->s0[i] = st->s0[i-1];
      st->s1[i] = st->s1[i-1];
      st->s2[i] = st->s2[i-1];
      st->s3[i] = st->s3[i-1];
      uint64_t tmp[4];
      tmp[0] = st->s0[i];
      tmp[1] = st->s1[i];
      tmp[2] = st->s2[i];
      tmp[3] = st->s3[i];
      xoshiro256pp_jump(tmp);
      st->s0[i] = tmp[0];
      st->s1[i] = tmp[1];
      st->s2[i] = tmp[2];
      st->s3[i] = tmp[3];
    }
  }
}

static void rand_init(randompack_rng *rng) {
  rng->buf_word = BUFSIZE;
  rng->buf_byte = 0;
  rng->last_error = 0;
  rng->usefullmantissa = false;
  normalize_state(rng);
  fill_stream_states_with_jump(rng);
}

static void rand_init_randomize(randompack_rng *rng) {
  if (rng->engine == CHACHA20) {
    uint8_t tmp[44];
    entropy_fill(rng, tmp, sizeof(tmp));
    for (int i = 0; i < 11; i++) {
      uint32_t v = (uint32_t)tmp[4*i] |
        ((uint32_t)tmp[4*i + 1] << 8) |
        ((uint32_t)tmp[4*i + 2] << 16) |
        ((uint32_t)tmp[4*i + 3] << 24);
      rng->state.u32[i] = v;
    }
    rng->state.u32[11] = 0;
  }
  else if (rng->engine == FAST) {
    uint64_t tmp[4];
    entropy_fill(rng, tmp, sizeof(tmp));
    rng->state.xo.s0[0] = tmp[0];
    rng->state.xo.s1[0] = tmp[1];
    rng->state.xo.s2[0] = tmp[2];
    rng->state.xo.s3[0] = tmp[3];
  }
  else {
    entropy_fill(rng, rng->state.u64, sizeof(rng->state.u64));
  }
  rand_init(rng);
}

#include "seed_seq_fe128.inc"

// Build an extended seed sequence: [TAG0, seed32, nkey+1, TAG1, spawn_key[0..nkey-1]]
// then seed seed_seq_fe128 from it and generate nw output words into w. Returns true on
// success, false on invalid args or allocation failure.
#define TAG0 314159265u  // π * 1e8
#define TAG1 271828182u  // e * 1e8
static bool seed_seq_seed(uint32_t *w, int nw, uint32_t seed32, uint32_t *spawn_key, int
                          nkey) {
  if (nkey > 0 && !spawn_key) return false;
  uint32_t *seq = 0;
  int nseq = nkey + 4;
  if (!ALLOC(seq, nseq)) return false;
  seq[0] = TAG0;
  seq[1] = seed32;
  seq[2] = nkey + 1;
  seq[3] = TAG1;
  if (nkey > 0) copy32(seq + 4, spawn_key, nkey);
  seed_seq_fe128 ss;
  seed_seq_fe128_seed(&ss, seq, nseq);
  seed_seq_fe128_generate(&ss, w, nw);
  FREE(seq);
  return true;
}

// static void store_u64_le(uint8_t *dst, uint64_t v) {
//   dst[0] = v;
//   dst[1] = v >> 8;
//   dst[2] = v >> 16;
//   dst[3] = v >> 24;
//   dst[4] = v >> 32;
//   dst[5] = v >> 40;
//   dst[6] = v >> 48;
//   dst[7] = v >> 56;
// }

static void set_state(uint64_t *state, int nstate, randompack_rng *rng) {  
  rng->buf_word = BUFSIZE;
  rng->buf_byte = 0;
  if (rng->engine == CHACHA20) {
    int k = 0;
    for (int i = 0; i < nstate; i++) {
      uint64_t v = state[i];
      rng->state.u32[k++] = (uint32_t)v;
      rng->state.u32[k++] = (uint32_t)(v >> 32);
    }
  }
  else if (rng->engine == FAST) {
    rng->state.xo.s0[0] = state[0];
    rng->state.xo.s1[0] = state[1];
    rng->state.xo.s2[0] = state[2];
    rng->state.xo.s3[0] = state[3];
  }
  else copy64(rng->state.u64, state, nstate);
}

static void philox_set_state(randompack_philox_ctr ctr, randompack_philox_key key,
  randompack_rng *rng) {
  uint64_t state[6] = {
    ctr.v[0], ctr.v[1], ctr.v[2], ctr.v[3],
    key.v[0], key.v[1]
  };
  set_state(state, LEN(state), rng);
}

static void squares_set_state(uint64_t ctr, uint64_t key, randompack_rng *rng) {
  uint64_t state[2] = {ctr, key};
  set_state(state, LEN(state), rng);
}

static void pcg64_set_state(uint64_t pcgstate, uint64_t inc, randompack_rng *rng) {
  uint64_t state[4] = {pcgstate, 0, inc, 0};
  set_state(state, LEN(state), rng);
}
