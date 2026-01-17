// -*- mode: c -*-
static inline void fill_csprng(randompack_rng *rng) {
  size_t need = BUFSIZE*sizeof(uint64_t);
  if (!crypto_random(rng->buf.u64, need)) {
    rng->last_error = "System CSPRNG failed (probably error reading /dev/random)";
    return;
  }
  rng->last_error = 0;
}
