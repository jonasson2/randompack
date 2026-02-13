// -*- C -*-

#include <stdint.h>
#include <stddef.h>

#if HAVE_NEON
#include <arm_neon.h>
#endif

// Convert uint32 RNG output to floats in [0,1) using 23-bit or 24-bit precision.
// 23-bit: k*2^-23 for k=0..2^23−1; 24-bit: k*2^-24 for k=0..2^24−1.
ALWAYS_INLINE void rand_float_8x23(float x[], size_t len, uint32_t u32[]) {
  size_t i = 0;
#if HAVE_NEON
  float32x4_t scale = vdupq_n_f32(0x1.0p-23f);
  for (; i + 3 < len; i += 4) {
    uint32x4_t r = vld1q_u32(u32 + i);
    r = vshrq_n_u32(r, 9);
    float32x4_t d = vcvtq_f32_u32(r);
    d = vmulq_f32(d, scale);
    vst1q_f32(x + i, d);
  }
#endif
  for (; i < len; i++) {
    x[i] = (float)(u32[i] >> 9)*0x1.0p-23f;
  }
}

ALWAYS_INLINE void rand_float_8x24(float x[], size_t len, uint32_t u32[]) {
  size_t i = 0;
#if HAVE_NEON
  float32x4_t scale = vdupq_n_f32(0x1.0p-24f);
  for (; i + 3 < len; i += 4) {
    uint32x4_t r = vld1q_u32(u32 + i);
    r = vshrq_n_u32(r, 8);
    float32x4_t d = vcvtq_f32_u32(r);
    d = vmulq_f32(d, scale);
    vst1q_f32(x + i, d);
  }
#endif
  for (; i < len; i++) {
    x[i] = U32_TO_FLOAT(u32[i]);
  }
}

ALWAYS_INLINE void rand_float(float x[], size_t len, randompack_rng *rng) {
#if defined(BUILD_AVX2)
  if (rng->cpu_has_avx2) {
    rand_float_avx2(x, len, rng);
    return;
  }
#endif
  int w = enter_u32_mode(rng);
  size_t i = 0;
  while (i < len) {
    fill_if_empty32(rng, &w);
    size_t remain = len - i;
    size_t avail = 2*BUFSIZE - (size_t)w;
    size_t take = remain < avail ? remain : avail;
    if (rng->usefullmantissa) {
      rand_float_8x24(x + i, take, rng->buf.u32 + (size_t)w);
    }
    else {
      rand_float_8x23(x + i, take, rng->buf.u32 + (size_t)w);
    }
    w += (int)take;
    i += take;
  }
  exit_u32_mode(rng, w);
}
