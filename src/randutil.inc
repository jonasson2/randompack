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
    x = (uint64_t)ts.tv_sec;
    x ^= ((uint64_t)ts.tv_nsec << 32);
    x ^= (uint64_t)(uintptr_t)rng;
    fill_bytes(buffer, (int)len, &x);
  }
}

static void rand_randomize(randompack_rng *rng) {
  if (rng->engine == CHACHA20) {
    key256_t key;
    nonce96_t nonce;
    ChaCha20_Ctx *ctx = (ChaCha20_Ctx *)rng->extra_state;
    entropy_fill(rng, key, sizeof(key));
    entropy_fill(rng, nonce, sizeof(nonce));
    ChaCha20_init(ctx, key, nonce, 0);
  }
  else {
    entropy_fill(rng, rng->state.u64, sizeof(rng->state.u64));
	 if (rng->engine == PHILOX) {
		philox_state *st = (philox_state *)rng->extra_state;
		entropy_fill(rng, &st->key, sizeof(st->key));
		st->idx = 4;
	 }
	 else if (rng->state.u64[0] == 0)
		rng->state.u64[0] = 1;
  }
  rng->buf_word = BUFFSIZE;
  rng->buf_byte = 0;
  rng->spare_norm = NAN;
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
  seq[2] = (uint32_t)(nkey + 1);
  seq[3] = TAG1;
  if (nkey > 0) copy32(seq + 4, spawn_key, nkey);
  seed_seq_fe128 ss;
  seed_seq_fe128_seed(&ss, seq, nseq);
  seed_seq_fe128_generate(&ss, w, nw);
  FREE(seq);
  return true;
}

static inline void store_le32(uint8_t *dst, uint32_t x) {
  dst[0] = (uint8_t)(x);
  dst[1] = (uint8_t)(x >> 8);
  dst[2] = (uint8_t)(x >> 16);
  dst[3] = (uint8_t)(x >> 24);
}

static void set_state(uint64_t *state, int nstate, randompack_rng *rng) {  
  rng->buf_word = BUFFSIZE;
  rng->buf_byte = 0;
  rng->spare_norm = INFINITY;
  if (rng->engine == CHACHA20) {
    uint32_t w[12];
    for (int i=0; i<6; i++) {
      w[2*i + 0] = (uint32_t)state[i];
      w[2*i + 1] = (uint32_t)(state[i] >> 32);
    }
	 assert(rng->extra_state);
    ChaCha20_Ctx *ctx = (ChaCha20_Ctx *)rng->extra_state;
    key256_t key;
    nonce96_t nonce;
    for (int i=0; i<8; i++) store_le32(&key[4*i], w[i]);  // for big-endian correctness
    for (int i=0; i<3; i++) store_le32(&nonce[4*i], w[8 + i]);  // ditto    
    ChaCha20_init(ctx, key, nonce, w[11]);
  }
  else if (rng->engine == PARKMILLER) {
	 rng->state.u32 = (uint32_t)state[0];
  }
  else {
	 int ncore = min(nstate, 4), nextra = nstate - 4;
	 copy64(rng->state.u64, state, ncore);
	 for (int i=ncore; i<4; i++) rng->state.u64[i] = 0;
	 if (nextra > 0) {
		assert(rng->extra_state);
		copy64(rng->extra_state, state + 4, nextra);
	 }
	 if (rng->engine == PHILOX) {
		philox_state *st = (philox_state *)rng->extra_state;
		st->idx = 4;
	 }	 
  }
}
