// -*- mode: c; -*-
static void fill_bytes(uint8_t *a, int len, uint64_t *x) {
  for (int k = 0; k < len; k += 8) {
    uint64_t mixed = rand_splitmix64(x);
    memcpy(a + k, &mixed, min(8, len - k));
  }
}

static void entropy_fill(randompack_rng *rng, void *buffer, size_t len) {
  bool ok = crypto_random(buffer, len);
  if (!ok) {
    struct timespec ts;
    uint64_t x;
    timespec_get(&ts, TIME_UTC);
    x = ts.tv_sec;
    x ^= ((uint64_t)ts.tv_nsec << 32);
    x ^= (uintptr_t)rng;
    fill_bytes(buffer, len, &x);
  }
}

static inline void normalize_state(randompack_rng *rng) {
  // Make state nonzero for xor-family, odd for cwg128 & pcg64.
  switch (rng->engine) {
  case X256PP:
  case X256SS:
  case XORO:
  case X128P:
    if (rng->state.u64[0] == 0) rng->state.u64[0] = 1;
    break;
  case CWG128:
    rng->state.u64[3] |= 1;
    break;
  case PCG64:
#if HAVE128
    rng->state.cwg.s |= 1;
#endif
    break;
  default:
    break;
  }
}

static inline void fill_stream_states_with_jump(randompack_rng *rng) {
  if (rng->engine == FAST) {
	 for (int i = 1; i < MAXSTREAMS; i++) {
		copy64(rng->state.ustream[i], rng->state.ustream[i-1], 4);
		xoshiro256pp_jump(rng->state.ustream[i]);
	 }
  }
}

static void rand_init(randompack_rng *rng) {
  rng->buf_word = BUFSIZE;
  rng->buf_byte = 0;
  rng->last_error = 0;
  normalize_state(rng);
  fill_stream_states_with_jump(rng);
}

static void rand_init_randomize(randompack_rng *rng) {
  if (rng->engine == CHACHA20) {
    entropy_fill(rng, &rng->state.u8[0],  32); // key (8 uints)
    entropy_fill(rng, &rng->state.u8[32], 12); // nonce (3 uints)
    rng->state.u32[11] = 0;                    // counter (1 uint)
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

static void store_u64_le(uint8_t *dst, uint64_t v) {
  dst[0] = v;
  dst[1] = v >> 8;
  dst[2] = v >> 16;
  dst[3] = v >> 24;
  dst[4] = v >> 32;
  dst[5] = v >> 40;
  dst[6] = v >> 48;
  dst[7] = v >> 56;
}

static void set_state(uint64_t *state, int nstate, randompack_rng *rng) {  
  rng->buf_word = BUFSIZE;
  rng->buf_byte = 0;
  if (rng->engine == CHACHA20) {
    for (int i = 0; i < nstate; i++)
      store_u64_le(rng->state.u8 + 8*i, state[i]);
  }
  else copy64(rng->state.u64, state, nstate);
}

static void philox_set_state(randompack_counter ctr, randompack_philox_key key,
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

#if HAVE128
static void pcg64_set_state(uint128_t pcgstate, uint128_t inc, randompack_rng *rng) {
  uint64_t state[4];
  copy64(state, &pcgstate, 2);
  copy64(state + 2, &inc, 2);
  set_state(state, LEN(state), rng);
}
#endif
