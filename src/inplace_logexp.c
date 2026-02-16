// -*- C -*-
// Sleef in-place log/exp helpers (AVX2).

#include <immintrin.h>
#include <math.h>
#include <stddef.h>

__m256d Sleef_expd4_u10avx2(__m256d d);
__m256d Sleef_logd4_u35avx2(__m256d d);
__m256 Sleef_expf8_u10avx2(__m256 d);
__m256 Sleef_logf8_u10avx2(__m256 d);

void sleef_exp_inplace(double *x, size_t len) {
  size_t i = 0;
  for (; i + 4 <= len; i += 4) {
    __m256d d = _mm256_loadu_pd(x + i);
    d = Sleef_expd4_u10avx2(d);
    _mm256_storeu_pd(x + i, d);
  }
  for (; i < len; i++) x[i] = exp(x[i]);
}

void sleef_log_inplace(double *x, size_t len) {
  size_t i = 0;
  for (; i + 4 <= len; i += 4) {
    __m256d d = _mm256_loadu_pd(x + i);
    d = Sleef_logd4_u35avx2(d);
    _mm256_storeu_pd(x + i, d);
  }
  for (; i < len; i++) x[i] = log(x[i]);
}

void sleef_expf_inplace(float *x, size_t len) {
  size_t i = 0;
  for (; i + 8 <= len; i += 8) {
    __m256 d = _mm256_loadu_ps(x + i);
    d = Sleef_expf8_u10avx2(d);
    _mm256_storeu_ps(x + i, d);
  }
  for (; i < len; i++) x[i] = expf(x[i]);
}

void sleef_logf_inplace(float *x, size_t len) {
  size_t i = 0;
  for (; i + 8 <= len; i += 8) {
    __m256 d = _mm256_loadu_ps(x + i);
    d = Sleef_logf8_u10avx2(d);
    _mm256_storeu_ps(x + i, d);
  }
  for (; i < len; i++) x[i] = logf(x[i]);
}
