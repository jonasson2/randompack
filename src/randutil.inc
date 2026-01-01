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

static void rand_randomize(randompack_rng *rng) {
  if (rng->engine == CHACHA20) {
    entropy_fill(rng, &rng->state.u8[0],  32); // key (8 uints)
    entropy_fill(rng, &rng->state.u8[32], 12); // nonce (3 uints)
    rng->state.u32[11] = 0;                    // counter (1 uint)
  }
  else {
    entropy_fill(rng, rng->state.u64, sizeof(rng->state.u64));
	 if (rng->state.u64[0] == 0) rng->state.u64[0] = 1;
  }
  rng->buf_word = BUFSIZE;
  rng->buf_byte = 0;
  rng->last_error = 0;
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

static void set_state(uint64_t *state, int nstate, randompack_rng *rng) {  
  rng->buf_word = BUFSIZE;
  rng->buf_byte = 0;
  copy64(rng->state.u64, state, nstate);
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

static void pcg64_set_state(uint128_t pcgstate, uint128_t inc, randompack_rng *rng) {
  uint64_t state[4];
  copy64(state, &pcgstate, 2);
  copy64(state + 2, &inc, 2);
  set_state(state, LEN(state), rng);
}

