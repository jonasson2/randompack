// -*- mode: c; -*-

static inline uint32_t rand_32bits(randompack_rng *rng) {
  if (rng->engine == PARKMILLER)
    return PM_rand_bits(rng);
  if (rng->buf64 == 0) { // get fresh 64 bit value from engine, use low 32 bits
    rng->buf64 = rng->next(rng);
    return (uint32_t)(rng->buf64 & 0xffffffffu);
  }
  uint32_t hi = (uint32_t)(rng->buf64 >> 32); // use high 32 bits and clear buffer
  rng->buf64 = 0;
  return hi;
}

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
  rng->buf64 = 0;
  rng->spare_norm = NAN;
  rng->last_error = 0;
}
