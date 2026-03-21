// -*- C -*-

ALWAYS_INLINE void shift_scale_double_inplace(double *x, size_t len, double shift,
  double scale, randompack_rng *rng) {
#if defined(BUILD_AVX2)
  if (rng->cpu_has_avx2) {
    shift_scale_double_avx2(x, len, shift, scale);
    return;
  }
#else
  (void)rng;
#endif
  for (size_t i = 0; i < len; i++) x[i] = shift + scale*x[i];
}

ALWAYS_INLINE void shift_scale_float_inplace(float *x, size_t len, float shift,
  float scale, randompack_rng *rng) {
#if defined(BUILD_AVX2)
  if (rng->cpu_has_avx2) {
    shift_scale_float_avx2(x, len, shift, scale);
    return;
  }
#else
  (void)rng;
#endif
  for (size_t i = 0; i < len; i++) x[i] = shift + scale*x[i];
}

static void scale_double_inplace(double *x, size_t len, double scale,
  randompack_rng *rng) {
  shift_scale_double_inplace(x, len, 0, scale, rng);
}

static void scale_float_inplace(float *x, size_t len, float scale,
  randompack_rng *rng) {
  shift_scale_float_inplace(x, len, 0, scale, rng);
}
