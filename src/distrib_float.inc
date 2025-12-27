// -*- mode: c; -*-

static void rand_float(float x[], size_t n, randompack_rng *rng) {
  for (size_t i = 0; i < n; i++) {
    uint32_t v = draw_u32(rng);
    x[i] = (v >> 8)*(float)0x1.0p-24;
  }
}

