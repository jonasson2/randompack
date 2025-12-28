// -*- mode: c; -*-

static void rand_float(float x[], size_t n, randompack_rng *rng) {
  for (size_t i = 0; i < n; i++) {
    uint32_t v = draw_u32(rng);
    x[i] = (v >> 8)*(float)0x1.0p-24;
  }
}

static inline void rand_normf(float x[], size_t n, randompack_rng *rng) {
  for (size_t i = 0; i < n; i++) {
    x[i] = random_standard_normal_f(rng);
  }
}

static inline void rand_expf(float x[], size_t n, float scale, randompack_rng *rng) {
  if (scale == 1.0f) {
    for (size_t i = 0; i < n; i++) x[i] = random_standard_exponential_f(rng);
  }
  else {
    for (size_t i = 0; i < n; i++) x[i] = random_standard_exponential_f(rng)*scale;
  }
}
