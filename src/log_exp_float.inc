// -*- c -*-
#if defined(USE_ACCEL_VV)
void vvexpf(float *y, const float *x, const int *n);
void vvlogf(float *y, const float *x, const int *n);
#endif

#if defined(BUILD_AVX2) || (defined(__aarch64__) && !defined(__APPLE__))
void sleef_expf_inplace(float *x, size_t len);
void sleef_logf_inplace(float *x, size_t len);
#endif

NEVER_INLINE void expf_inplace(float *x, size_t len, randompack_rng *rng) {
  if (!rng->bitexact) {
#if defined(USE_ACCEL_VV)
    for (size_t i = 0; i < len; ) {
      size_t chunk = len - i > (size_t)INT_MAX ? (size_t)INT_MAX : len - i;
      int n = (int)chunk;
      vvexpf(x + i, x + i, &n);
      i += chunk;
    }
    return;
#elif defined(BUILD_AVX2)
#if defined(_WIN32)
    for (size_t i = 0; i < len; i++) x[i] = expf(x[i]);
    return;
#else
    if (rng->cpu_has_avx2) {
      sleef_expf_inplace(x, len);
      return;
    }
    for (size_t i = 0; i < len; i++) x[i] = expf(x[i]);
    return;
#endif
#elif defined(__aarch64__) && !defined(__APPLE__)
    sleef_expf_inplace(x, len);
    return;
#else
    for (size_t i = 0; i < len; i++) x[i] = expf(x[i]);
    return;
#endif
  }
  for (size_t i = 0; i < len; i++) x[i] = openlibm_expf(x[i]);
}

NEVER_INLINE void logf_inplace(float *x, size_t len, randompack_rng *rng) {
  if (!rng->bitexact) {
#if defined(USE_ACCEL_VV)
    for (size_t i = 0; i < len; ) {
      size_t chunk = len - i > (size_t)INT_MAX ? (size_t)INT_MAX : len - i;
      int n = (int)chunk;
      vvlogf(x + i, x + i, &n);
      i += chunk;
    }
    return;
#elif defined(BUILD_AVX2)
#if defined(_WIN32)
    for (size_t i = 0; i < len; i++) x[i] = logf(x[i]);
    return;
#else
    if (rng->cpu_has_avx2) {
      sleef_logf_inplace(x, len);
      return;
    }
    for (size_t i = 0; i < len; i++) x[i] = logf(x[i]);
    return;
#endif
// #elif defined(__aarch64__) && !defined(__APPLE__)
//     sleef_logf_inplace(x, len);
//     return;
#else
    for (size_t i = 0; i < len; i++) x[i] = logf(x[i]);
    return;
#endif
  }
  for (size_t i = 0; i < len; i++) x[i] = openlibm_logf(x[i]);
}
