// -*- C -*-
// Sleef in-place log/exp helpers.

#include <math.h>
#include <stddef.h>

#if defined(_MSC_VER)
  #define HIDDEN
#elif defined(__GNUC__) || defined(__clang__)
  #define HIDDEN __attribute__((visibility("hidden")))
#else
  #define HIDDEN
#endif

#if defined(__x86_64__) || defined(_M_X64)

#include <immintrin.h>

__m256d Sleef_expd4_u10avx2(__m256d d);
__m256d Sleef_logd4_u35avx2(__m256d d);
__m256 Sleef_expf8_u10avx2(__m256 d);
__m256 Sleef_logf8_u10avx2(__m256 d);

HIDDEN void sleef_exp_inplace(double *x, size_t len) {
  size_t i = 0;
  for (; i + 4 <= len; i += 4) {
    __m256d d = _mm256_loadu_pd(x + i);
    d = Sleef_expd4_u10avx2(d);
    _mm256_storeu_pd(x + i, d);
  }
  for (; i < len; i++) x[i] = exp(x[i]);
}

HIDDEN void sleef_log_inplace(double *x, size_t len) {
  size_t i = 0;
  for (; i + 4 <= len; i += 4) {
    __m256d d = _mm256_loadu_pd(x + i);
    d = Sleef_logd4_u35avx2(d);
    _mm256_storeu_pd(x + i, d);
  }
  for (; i < len; i++) x[i] = log(x[i]);
}

HIDDEN void sleef_expf_inplace(float *x, size_t len) {
  size_t i = 0;
  for (; i + 8 <= len; i += 8) {
    __m256 d = _mm256_loadu_ps(x + i);
    d = Sleef_expf8_u10avx2(d);
    _mm256_storeu_ps(x + i, d);
  }
  for (; i < len; i++) x[i] = expf(x[i]);
}

HIDDEN void sleef_logf_inplace(float *x, size_t len) {
  size_t i = 0;
  for (; i + 8 <= len; i += 8) {
    __m256 d = _mm256_loadu_ps(x + i);
    d = Sleef_logf8_u10avx2(d);
    _mm256_storeu_ps(x + i, d);
  }
  for (; i < len; i++) x[i] = logf(x[i]);
}

#elif (defined(__aarch64__) || defined(_M_ARM64)) && !defined(__APPLE__)

#include <arm_neon.h>

float64x2_t Sleef_expd2_u10advsimd(float64x2_t d);
float64x2_t Sleef_logd2_u35advsimd(float64x2_t d);
float32x4_t Sleef_expf4_u10advsimd(float32x4_t d);
float32x4_t Sleef_logf4_u10advsimd(float32x4_t d);

HIDDEN void sleef_exp_inplace(double *x, size_t len) {
  size_t i = 0;
  for (; i + 2 <= len; i += 2) {
    float64x2_t d = vld1q_f64(x + i);
    d = Sleef_expd2_u10advsimd(d);
    vst1q_f64(x + i, d);
  }
  for (; i < len; i++) x[i] = exp(x[i]);
}

HIDDEN void sleef_log_inplace(double *x, size_t len) {
  size_t i = 0;
  for (; i + 2 <= len; i += 2) {
    float64x2_t d = vld1q_f64(x + i);
    d = Sleef_logd2_u35advsimd(d);
    vst1q_f64(x + i, d);
  }
  for (; i < len; i++) x[i] = log(x[i]);
}

HIDDEN void sleef_expf_inplace(float *x, size_t len) {
  size_t i = 0;
  for (; i + 4 <= len; i += 4) {
    float32x4_t d = vld1q_f32(x + i);
    d = Sleef_expf4_u10advsimd(d);
    vst1q_f32(x + i, d);
  }
  for (; i < len; i++) x[i] = expf(x[i]);
}

HIDDEN void sleef_logf_inplace(float *x, size_t len) {
  size_t i = 0;
  for (; i + 4 <= len; i += 4) {
    float32x4_t d = vld1q_f32(x + i);
    d = Sleef_logf4_u10advsimd(d);
    vst1q_f32(x + i, d);
  }
  for (; i < len; i++) x[i] = logf(x[i]);
}

#else

HIDDEN void sleef_exp_inplace(double *x, size_t len) {
  for (size_t i = 0; i < len; i++) x[i] = exp(x[i]);
}

HIDDEN void sleef_log_inplace(double *x, size_t len) {
  for (size_t i = 0; i < len; i++) x[i] = log(x[i]);
}

HIDDEN void sleef_expf_inplace(float *x, size_t len) {
  for (size_t i = 0; i < len; i++) x[i] = expf(x[i]);
}

HIDDEN void sleef_logf_inplace(float *x, size_t len) {
  for (size_t i = 0; i < len; i++) x[i] = logf(x[i]);
}
#endif
