// -*- C -*-

#include "randompack_internal.h"

#if !defined(__x86_64__) && !defined(_M_X64)

bool cpu_has_avx512(void) {
  return false;
}

void fill_sfc64simd_avx512(uint64_t *buf, size_t len, randompack_state *state) {
  (void)buf;
  (void)len;
  (void)state;
}

void rand_dble_avx512(double x[], size_t len, randompack_rng *rng) {
  (void)x;
  (void)len;
  (void)rng;
}

void shift_scale_double_avx512(double x[], size_t len, double shift, double scale) {
  for (size_t i = 0; i < len; i++) x[i] = shift + scale*x[i];
}

void shift_scale_float_avx512(float x[], size_t len, float shift, float scale) {
  for (size_t i = 0; i < len; i++) x[i] = shift + scale*x[i];
}

void scale_double_avx512(double x[], size_t len, double scale) {
  shift_scale_double_avx512(x, len, 0, scale);
}

void scale_float_avx512(float x[], size_t len, float scale) {
  shift_scale_float_avx512(x, len, 0, scale);
}

#else

#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <cpuid.h>
#endif

#include <immintrin.h>

#ifndef __m512i_u
typedef __m512i __m512i_u;
#endif

#include <string.h>
#include "buffer_draw.inc"

#if defined(_MSC_VER)
#define HIDDEN
#elif defined(__GNUC__) || defined(__clang__)
#define HIDDEN __attribute__((visibility("hidden")))
#else
#define HIDDEN
#endif

HIDDEN bool cpu_has_avx512(void) {
#if defined(_MSC_VER)
  int info[4];
  __cpuid(info, 1);
  int osxsave = (info[2] & (1 << 27)) != 0;
  int avx = (info[2] & (1 << 28)) != 0;
  if (!(osxsave && avx)) return false;
  unsigned long long xcr = _xgetbv(0);
  if ((xcr & 0xe6) != 0xe6) return false;
  __cpuidex(info, 7, 0);
  return (info[1] & (1 << 16)) != 0 && (info[1] & (1 << 17)) != 0;
#elif defined(__GNUC__) || defined(__clang__)
  unsigned int eax, ebx, ecx, edx;
  if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx)) return false;
  if (!(ecx & (1 << 27))) return false;
  if (!(ecx & (1 << 28))) return false;
  unsigned int xcr0_lo, xcr0_hi;
  __asm__ volatile ("xgetbv" : "=a"(xcr0_lo), "=d"(xcr0_hi) : "c"(0));
  if ((xcr0_lo & 0xe6) != 0xe6) return false;
  if (__get_cpuid_max(0, 0) < 7) return false;
  __cpuid_count(7, 0, eax, ebx, ecx, edx);
  return (ebx & (1 << 16)) != 0 && (ebx & (1 << 17)) != 0;
#else
  return false;
#endif
}

HIDDEN void fill_sfc64simd_avx512(uint64_t *buf, size_t len,
  randompack_state *state) {
  uint64_t *out = buf;
  xo256 *st = &state->xo;
  __m512i a = _mm512_loadu_si512((const void *)&st->s0[0]);
  __m512i b = _mm512_loadu_si512((const void *)&st->s1[0]);
  __m512i c = _mm512_loadu_si512((const void *)&st->s2[0]);
  __m512i d = _mm512_loadu_si512((const void *)&st->s3[0]);
  __m512i one = _mm512_set1_epi64(1);
  size_t i = 0;
  for (; i < len; i += 8) {
    __m512i t;
    __m512i r = _mm512_add_epi64(_mm512_add_epi64(a, b), d);
    d = _mm512_add_epi64(d, one);
    t = _mm512_xor_si512(b, _mm512_srli_epi64(b, 11));
    b = _mm512_add_epi64(c, _mm512_slli_epi64(c, 3));
    c = _mm512_add_epi64(
      _mm512_or_si512(_mm512_slli_epi64(c, 24), _mm512_srli_epi64(c, 40)),
      r
    );
    a = t;
    _mm512_storeu_si512((void *)(out + i), r);
  }
  _mm512_storeu_si512((void *)&st->s0[0], a);
  _mm512_storeu_si512((void *)&st->s1[0], b);
  _mm512_storeu_si512((void *)&st->s2[0], c);
  _mm512_storeu_si512((void *)&st->s3[0], d);
}

HIDDEN void shift_scale_double_avx512(double x[], size_t len, double shift,
  double scale) {
  size_t i = 0;
  __m512d s = _mm512_set1_pd(scale);
  __m512d b = _mm512_set1_pd(shift);
  for (; i + 32 <= len; i += 32) {
    __m512d v0 = _mm512_loadu_pd(x + i);
    __m512d v1 = _mm512_loadu_pd(x + i + 8);
    __m512d v2 = _mm512_loadu_pd(x + i + 16);
    __m512d v3 = _mm512_loadu_pd(x + i + 24);
    v0 = _mm512_add_pd(_mm512_mul_pd(v0, s), b);
    v1 = _mm512_add_pd(_mm512_mul_pd(v1, s), b);
    v2 = _mm512_add_pd(_mm512_mul_pd(v2, s), b);
    v3 = _mm512_add_pd(_mm512_mul_pd(v3, s), b);
    _mm512_storeu_pd(x + i, v0);
    _mm512_storeu_pd(x + i + 8, v1);
    _mm512_storeu_pd(x + i + 16, v2);
    _mm512_storeu_pd(x + i + 24, v3);
  }
  for (; i + 8 <= len; i += 8) {
    __m512d v = _mm512_loadu_pd(x + i);
    v = _mm512_add_pd(_mm512_mul_pd(v, s), b);
    _mm512_storeu_pd(x + i, v);
  }
  for (; i < len; i++) x[i] = shift + scale*x[i];
}

HIDDEN void affine_double_avx512(double x[], size_t len, double shift,
  double scale, double hi) {
  size_t i = 0;
  __m512d s = _mm512_set1_pd(scale);
  __m512d b = _mm512_set1_pd(shift);
#if !defined(FP_FAST_FMA)
  __m512d h = _mm512_set1_pd(hi);
#endif
  for (; i + 32 <= len; i += 32) {
    __m512d v0 = _mm512_loadu_pd(x + i);
    __m512d v1 = _mm512_loadu_pd(x + i + 8);
    __m512d v2 = _mm512_loadu_pd(x + i + 16);
    __m512d v3 = _mm512_loadu_pd(x + i + 24);
    v0 = _mm512_add_pd(_mm512_mul_pd(v0, s), b);
    v1 = _mm512_add_pd(_mm512_mul_pd(v1, s), b);
    v2 = _mm512_add_pd(_mm512_mul_pd(v2, s), b);
    v3 = _mm512_add_pd(_mm512_mul_pd(v3, s), b);
#if !defined(FP_FAST_FMA)
    v0 = _mm512_min_pd(v0, h);
    v1 = _mm512_min_pd(v1, h);
    v2 = _mm512_min_pd(v2, h);
    v3 = _mm512_min_pd(v3, h);
#endif
    _mm512_storeu_pd(x + i, v0);
    _mm512_storeu_pd(x + i + 8, v1);
    _mm512_storeu_pd(x + i + 16, v2);
    _mm512_storeu_pd(x + i + 24, v3);
  }
  for (; i + 8 <= len; i += 8) {
    __m512d v = _mm512_loadu_pd(x + i);
    v = _mm512_add_pd(_mm512_mul_pd(v, s), b);
#if !defined(FP_FAST_FMA)
    v = _mm512_min_pd(v, h);
#endif
    _mm512_storeu_pd(x + i, v);
  }
  for (; i < len; i++) {
    double y = shift + scale*x[i];
#if !defined(FP_FAST_FMA)
    y = y > hi ? hi : y;
#endif
    x[i] = y;
  }
}

HIDDEN void shift_scale_float_avx512(float x[], size_t len, float shift,
  float scale) {
  size_t i = 0;
  __m512 s = _mm512_set1_ps(scale);
  __m512 b = _mm512_set1_ps(shift);
  for (; i + 64 <= len; i += 64) {
    __m512 v0 = _mm512_loadu_ps(x + i);
    __m512 v1 = _mm512_loadu_ps(x + i + 16);
    __m512 v2 = _mm512_loadu_ps(x + i + 32);
    __m512 v3 = _mm512_loadu_ps(x + i + 48);
    v0 = _mm512_add_ps(_mm512_mul_ps(v0, s), b);
    v1 = _mm512_add_ps(_mm512_mul_ps(v1, s), b);
    v2 = _mm512_add_ps(_mm512_mul_ps(v2, s), b);
    v3 = _mm512_add_ps(_mm512_mul_ps(v3, s), b);
    _mm512_storeu_ps(x + i, v0);
    _mm512_storeu_ps(x + i + 16, v1);
    _mm512_storeu_ps(x + i + 32, v2);
    _mm512_storeu_ps(x + i + 48, v3);
  }
  for (; i + 16 <= len; i += 16) {
    __m512 v = _mm512_loadu_ps(x + i);
    v = _mm512_add_ps(_mm512_mul_ps(v, s), b);
    _mm512_storeu_ps(x + i, v);
  }
  for (; i < len; i++) x[i] = shift + scale*x[i];
}

HIDDEN void scale_double_avx512(double x[], size_t len, double scale) {
  shift_scale_double_avx512(x, len, 0, scale);
}

HIDDEN void scale_float_avx512(float x[], size_t len, float scale) {
  shift_scale_float_avx512(x, len, 0, scale);
}

HIDDEN void rand_dble_avx512(double x[], size_t len, randompack_rng *rng) {
  int w = enter_u64_mode(rng);
  size_t i = 0;
  while (i < len) {
    fill_if_empty(rng, &w);
    size_t remain = len - i;
    size_t avail = BUFSIZE - (size_t)w;
    size_t take = remain < avail ? remain : avail;
    uint64_t *u64 = rng->buf.u64 + (size_t)w;
    if (rng->usefullmantissa) {
      for (size_t j = 0; j < take; j++) x[i + j] = U64_TO_DOUBLE(u64[j]);
    }
    else {
      size_t j = 0;
      __m512i expo = _mm512_set1_epi64(0x3ff0000000000000ULL);
      __m512d one = _mm512_set1_pd(1);
      for (; j + 7 < take; j += 8) {
        __m512i r = _mm512_loadu_si512((const void *)(u64 + j));
        r = _mm512_srli_epi64(r, 12);
        r = _mm512_or_si512(r, expo);
        __m512d d = _mm512_castsi512_pd(r);
        d = _mm512_sub_pd(d, one);
        _mm512_storeu_pd(x + i + j, d);
      }
      for (; j < take; j++) {
        uint64_t bits = (u64[j] >> 12) | 0x3ff0000000000000ULL;
        double d;
        memcpy(&d, &bits, sizeof(d));
        x[i + j] = d - 1;
      }
    }
    w += (int)take;
    i += take;
  }
  exit_u64_mode(rng, w);
}

#endif
