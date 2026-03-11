// -*- mode: c; -*-
#include <time.h>

static int get_state_words(randompack_rng *rng) {
  rng_entry *ent = find_entry(rng->engine);
  int nwords = ent ? ent->state_words : 1;
  return nwords > 0 ? nwords : 1;
}

static inline uint64_t rand_splitmix64(uint64_t *x) {
  uint64_t z = (*x += 0x9E3779B97F4A7C15ULL);
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
  return z ^ (z >> 31);
}

static void fill_splitmix64(uint8_t *a, int len, uint64_t *x) {
  for (int k = 0; k < len; k += 8) {
    uint64_t mixed = rand_splitmix64(x);
    memcpy(a + k, &mixed, min(8, len - k));
  }
}

static void fill_entropy(void *buffer, int len) {
  bool ok = crypto_random(buffer, (size_t) len);
  if (!ok) {  // fallback on esoteric systems
    uint64_t x = clock_nsec();
    x ^= (uintptr_t)buffer;
    fill_splitmix64(buffer, len, &x);
  }
}

static inline void get_stream(uint64_t stream_state[4], int i, randompack_rng *rng) {
  xo256 *st = &rng->state.xo;
  stream_state[0] = st->s0[i];
  stream_state[1] = st->s1[i];
  stream_state[2] = st->s2[i];
  stream_state[3] = st->s3[i];
}

static inline void set_stream(int i, const uint64_t stream_state[4], randompack_rng *rng) {
  xo256 *st = &rng->state.xo;
  st->s0[i] = stream_state[0];
  st->s1[i] = stream_state[1];
  st->s2[i] = stream_state[2];
  st->s3[i] = stream_state[3];
}

static inline void scatter_to_stream0(randompack_rng *rng) {
  // used for SIMD streams
  xo256 *st = &rng->state.xo;
  uint64_t *u64 = rng->state.u64;
  st->s0[0] = u64[0];
  st->s1[0] = u64[1];
  st->s2[0] = u64[2];
  st->s3[0] = u64[3];
}

static inline void gather_from_stream0(randompack_rng *rng) {
  xo256 *st = &rng->state.xo;
  uint64_t *u64 = rng->state.u64;
  u64[0] = st->s0[0];
  u64[1] = st->s1[0];
  u64[2] = st->s2[0];
  u64[3] = st->s3[0];
}

static inline void x256ppsimd_jump(int p, randompack_rng *rng) {
  for (int i=0; i<4; i++) {
    uint64_t stream_state[4];
    get_stream(stream_state, i, rng);
    xoshiro256_jump(stream_state, p);
    set_stream(i, stream_state, rng);
  }
}

static inline void normalize_state(randompack_rng *rng) {
  // Make state nonzero for xor-family and ranlux, odd for cwg128 & pcg64.
  int nwords = get_state_words(rng);
  switch (rng->engine) {
  case X256PP:
  case X256SS:
  case FAST:
  case XORO:
  case X128P:
  case RANLUXPP:
    if (nwords > 0 && all_zero_state(rng->state.u64, nwords))
      rng->state.u64[0] = 1;
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
  for (int i=1; i<4; i++) {
    uint64_t stream_state[4];
    get_stream(stream_state, i-1, rng);
    xoshiro256_jump(stream_state, 254);
    set_stream(i, stream_state, rng);
  }
}

static void rand_init(randompack_rng *rng) {
  rng->buf_word = BUFSIZE;
  rng->buf_byte = 0;
  rng->last_error = 0;
  rng->usefullmantissa = false;
  rng->bitexact = false;
}

static void rand_randomize(randompack_rng *rng) {
  if (rng->engine == CHACHA20) {
    uint8_t tmp[44];
    fill_entropy(tmp, sizeof(tmp));
    for (int i = 0; i < 11; i++) {
      uint32_t v = (uint32_t)tmp[4*i] |
        ((uint32_t)tmp[4*i + 1] << 8) |
        ((uint32_t)tmp[4*i + 2] << 16) |
        ((uint32_t)tmp[4*i + 3] << 24);
      rng->state.u32[i] = v;
    }
    rng->state.u32[11] = 0;
  }
  else {
    int nwords = get_state_words(rng);
    fill_entropy(rng->state.u64, nwords*sizeof(uint64_t));
    normalize_state(rng);
    if (rng->engine == FAST) {
      scatter_to_stream0(rng);
      fill_stream_states_with_jump(rng);
    }
  }
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
  int nwords = get_state_words(rng);
  assert(nstate == nwords);
  if (rng->engine == CHACHA20) {
    int k = 0;
    for (int i = 0; i < nstate; i++) {
      uint64_t v = state[i];
      rng->state.u32[k++] = (uint32_t)v;
      rng->state.u32[k++] = (uint32_t)(v >> 32);
    }
  }
  else {
    copy64(rng->state.u64, state, nstate);
    if (rng->engine == FAST) {
      scatter_to_stream0(rng);
      fill_stream_states_with_jump(rng);
    }
  }
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
