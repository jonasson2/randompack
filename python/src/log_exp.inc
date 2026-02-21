// -*- c -*-
#if defined(USE_ACCEL_VV)
void vvexp(double *y, const double *x, const int *n);
void vvlog(double *y, const double *x, const int *n);
void vvexpf(float *y, const float *x, const int *n);
void vvlogf(float *y, const float *x, const int *n);
#endif

#if defined(BUILD_AVX2)
void sleef_exp_inplace(double *x, size_t len);
void sleef_log_inplace(double *x, size_t len);
void sleef_expf_inplace(float *x, size_t len);
void sleef_logf_inplace(float *x, size_t len);
#endif

static inline void exp_inplace(double *x, size_t len, randompack_rng *rng) {
  if (!rng->bitexact) {
#if defined(USE_ACCEL_VV)
    for (size_t i = 0; i < len; ) {
      size_t chunk = len - i > (size_t)INT_MAX ? (size_t)INT_MAX : len - i;
      int n = (int)chunk;
      vvexp(x + i, x + i, &n);
      i += chunk;
    }
    return;
#elif defined(BUILD_AVX2)
    if (rng->cpu_has_avx2) {
      sleef_exp_inplace(x, len);
      return;
    }
#endif
    for (size_t i = 0; i < len; i++) x[i] = exp(x[i]);
    return;
  }
  for (size_t i = 0; i < len; i++) x[i] = openlibm_exp(x[i]);
}

static inline void log_inplace(double *x, size_t len, randompack_rng *rng) {
  if (!rng->bitexact) {
#if defined(USE_ACCEL_VV)
    for (size_t i = 0; i < len; ) {
      size_t chunk = len - i > (size_t)INT_MAX ? (size_t)INT_MAX : len - i;
      int n = (int)chunk;
      vvlog(x + i, x + i, &n);
      i += chunk;
    }
    return;
#elif defined(BUILD_AVX2)
    if (rng->cpu_has_avx2) {
      sleef_log_inplace(x, len);
      return;
    }
#endif
    for (size_t i = 0; i < len; i++) x[i] = log(x[i]);
    return;
  }
  for (size_t i = 0; i < len; i++) x[i] = openlibm_log(x[i]);
}

static inline void expf_inplace(float *x, size_t len, randompack_rng *rng) {
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
    if (rng->cpu_has_avx2) {
      sleef_expf_inplace(x, len);
      return;
    }
#endif
    for (size_t i = 0; i < len; i++) x[i] = expf(x[i]);
    return;
  }
  for (size_t i = 0; i < len; i++) x[i] = openlibm_expf(x[i]);
}

static inline void logf_inplace(float *x, size_t len, randompack_rng *rng) {
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
    if (rng->cpu_has_avx2) {
      sleef_logf_inplace(x, len);
      return;
    }
#endif
    for (size_t i = 0; i < len; i++) x[i] = logf(x[i]);
    return;
  }
  for (size_t i = 0; i < len; i++) x[i] = openlibm_logf(x[i]);
}
