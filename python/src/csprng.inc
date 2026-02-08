// -*- mode: c -*-
static inline void fill_csprng(randompack_rng *rng, size_t len) {
  size_t need = len*sizeof(uint64_t);
  if (!crypto_random(rng->buf.u64 + rng->buf_word, need)) {
    rng->last_error = "System CSPRNG failed (probably error reading /dev/random)";
    return;
  }
  rng->last_error = 0;
}
