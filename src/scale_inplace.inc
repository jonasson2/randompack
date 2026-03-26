// -*- C -*-

#if HAVE_NEON
#include <arm_neon.h>
#endif

ALWAYS_INLINE void shift_scale_double_inplace(double *x, size_t len, double shift,
  double scale, randompack_rng *rng) {
#if defined(BUILD_AVX512)
  if (rng->cpu_has_avx512) {
    shift_scale_double_avx512(x, len, shift, scale);
    return;
  }
#endif
#if defined(BUILD_AVX2)
  if (rng->cpu_has_avx2) {
    shift_scale_double_avx2(x, len, shift, scale);
    return;
  }
#endif
#if HAVE_NEON
  size_t i = 0;
  float64x2_t s = vdupq_n_f64(scale);
  float64x2_t b = vdupq_n_f64(shift);
  for (; i + 4 <= len; i += 4) {
    float64x2_t v0 = vld1q_f64(x + i);
    float64x2_t v1 = vld1q_f64(x + i + 2);
    v0 = vaddq_f64(vmulq_f64(v0, s), b);
    v1 = vaddq_f64(vmulq_f64(v1, s), b);
    vst1q_f64(x + i, v0);
    vst1q_f64(x + i + 2, v1);
  }
  for (; i + 2 <= len; i += 2) {
    float64x2_t v = vld1q_f64(x + i);
    v = vaddq_f64(vmulq_f64(v, s), b);
    vst1q_f64(x + i, v);
  }
#if defined(FP_FAST_FMA)
  for (; i < len; i++) x[i] = fma(scale, x[i], shift);
#else
  for (; i < len; i++) x[i] = shift + scale*x[i];
#endif
  return;
#endif
  (void)rng;
#if defined(FP_FAST_FMA)
  for (size_t i = 0; i < len; i++) x[i] = fma(scale, x[i], shift);
#else
  for (size_t i = 0; i < len; i++) x[i] = shift + scale*x[i];
#endif
}

ALWAYS_INLINE void shift_scale_float_inplace(float *x, size_t len, float shift,
  float scale, randompack_rng *rng) {
#if defined(BUILD_AVX512)
  if (rng->cpu_has_avx512) {
    shift_scale_float_avx512(x, len, shift, scale);
    return;
  }
#endif
#if defined(BUILD_AVX2)
  if (rng->cpu_has_avx2) {
    shift_scale_float_avx2(x, len, shift, scale);
    return;
  }
#endif
#if HAVE_NEON
  size_t i = 0;
  float32x4_t s = vdupq_n_f32(scale);
  float32x4_t b = vdupq_n_f32(shift);
  for (; i + 8 <= len; i += 8) {
    float32x4_t v0 = vld1q_f32(x + i);
    float32x4_t v1 = vld1q_f32(x + i + 4);
    v0 = vaddq_f32(vmulq_f32(v0, s), b);
    v1 = vaddq_f32(vmulq_f32(v1, s), b);
    vst1q_f32(x + i, v0);
    vst1q_f32(x + i + 4, v1);
  }
  for (; i + 4 <= len; i += 4) {
    float32x4_t v = vld1q_f32(x + i);
    v = vaddq_f32(vmulq_f32(v, s), b);
    vst1q_f32(x + i, v);
  }
#if defined(FP_FAST_FMA)
  for (; i < len; i++) x[i] = fmaf(scale, x[i], shift);
#else
  for (; i < len; i++) x[i] = shift + scale*x[i];
#endif
  return;
#endif
  (void)rng;
#if defined(FP_FAST_FMA)
  for (size_t i = 0; i < len; i++) x[i] = fmaf(scale, x[i], shift);
#else
  for (size_t i = 0; i < len; i++) x[i] = shift + scale*x[i];
#endif
}

static void scale_double_inplace(double *x, size_t len, double scale,
  randompack_rng *rng) {
  shift_scale_double_inplace(x, len, 0, scale, rng);
}

static void scale_float_inplace(float *x, size_t len, float scale,
  randompack_rng *rng) {
  shift_scale_float_inplace(x, len, 0, scale, rng);
}
