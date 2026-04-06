// -*- C -*-

#if HAVE_NEON
#include <arm_neon.h>
#endif

ALWAYS_INLINE void shift_scale_float_inplace(float *x, size_t len, float shift,
  float scale, randompack_rng *rng) {
  if (!rng->bitexact) {
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
#if HAVE_NEON && defined(__aarch64__)
    size_t i = 0;
    float32x4_t s = vdupq_n_f32(scale);
    float32x4_t b = vdupq_n_f32(shift);
    for (; i + 8 <= len; i += 8) {
      float32x4_t v0 = vld1q_f32(x + i);
      float32x4_t v1 = vld1q_f32(x + i + 4);
      v0 = vfmaq_f32(b, v0, s);
      v1 = vfmaq_f32(b, v1, s);
      vst1q_f32(x + i, v0);
      vst1q_f32(x + i + 4, v1);
    }
    for (; i + 4 <= len; i += 4) {
      float32x4_t v = vld1q_f32(x + i);
      v = vfmaq_f32(b, v, s);
      vst1q_f32(x + i, v);
    }
    for (; i < len; i++) x[i] = fmaf(scale, x[i], shift);
    return;
#endif
  }
  (void)rng;
  for (size_t i = 0; i < len; i++) x[i] = shift + scale*x[i];
}

static void scale_float_inplace(float *x, size_t len, float scale,
  randompack_rng *rng) {
  shift_scale_float_inplace(x, len, 0, scale, rng);
}
