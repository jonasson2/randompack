// -*- mode: c -*-
static uint64_t csprng(randompack_rng *rng) {
  // Cryptographically secure pseudo-random-number-generator
  
  uint64_t x;

  if (!crypto_random(&x, sizeof x)) {  // platform dependent function
    rng->last_error = "system CSPRNG failed";
    return 0;
  }

  rng->last_error = 0;
  return x;
}