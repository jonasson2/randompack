// -*- C -*-

#include "randompack_internal.h"

#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <cpuid.h>
#endif

#include <immintrin.h>
#include <string.h>
#include "buffer_draw.inc"

#if defined(_MSC_VER)
#define HIDDEN
#elif defined(__GNUC__) || defined(__clang__)
#define HIDDEN __attribute__((visibility("hidden")))
#else
#define HIDDEN
#endif

#if defined(RANDOMPACK_TEST_HOOKS)
#if defined(__GNUC__) || defined(__clang__)
#define TEST_HOOK __attribute__((visibility("default")))
#else
#define TEST_HOOK
#endif
static int avx2_used = 0;
TEST_HOOK int randompack_avx2_used(void) {
  return avx2_used;
}
TEST_HOOK void randompack_avx2_reset(void) {
  avx2_used = 0;
}
#endif

HIDDEN bool cpu_has_avx2(void) {
#if defined(_MSC_VER)
  int info[4];
  __cpuid(info, 1);
  int osxsave = (info[2] & (1 << 27)) != 0;
  int avx = (info[2] & (1 << 28)) != 0;
  if (!(osxsave && avx)) return false;
  unsigned long long xcr = _xgetbv(0);
  if ((xcr & 0x6) != 0x6) return false;
  __cpuidex(info, 7, 0);
  return (info[1] & (1 << 5)) != 0;
#elif defined(__GNUC__) || defined(__clang__)
  unsigned int eax, ebx, ecx, edx;
  if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx)) return false;
  if (!(ecx & (1 << 27))) return false;
  if (!(ecx & (1 << 28))) return false;
  unsigned int xcr0_lo, xcr0_hi;
  __asm__ volatile ("xgetbv" : "=a"(xcr0_lo), "=d"(xcr0_hi) : "c"(0));
  if ((xcr0_lo & 0x6) != 0x6) return false;
  if (!__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) return false;
  return (ebx & (1 << 5)) != 0;
#else
  return false;
#endif
}

#define VEC_T __m256i
#define VEC_LOAD(p) _mm256_loadu_si256((const __m256i_u *)(p))
#define VEC_STORE(p,x) _mm256_storeu_si256((__m256i_u *)(p),(x))
#define VEC_ADD(a,b) _mm256_add_epi64((a),(b))
#define VEC_XOR(a,b) _mm256_xor_si256((a),(b))
#define VEC_SHL17(a) _mm256_slli_epi64((a),17)
#define VEC_ROTL23(x) _mm256_or_si256( \
  _mm256_slli_epi64((x),23), _mm256_srli_epi64((x),64-23))
#define VEC_ROTL45(x) _mm256_or_si256( \
  _mm256_slli_epi64((x),45), _mm256_srli_epi64((x),64-45))

#define FAST_STEP_VEC(s0,s1,s2,s3,outv) do { \
  VEC_T r_; \
  VEC_T t_; \
  r_ = VEC_ADD(VEC_ROTL23(VEC_ADD((s0),(s3))),(s0)); \
  t_ = VEC_SHL17((s1)); \
  (s2) = VEC_XOR((s2),(s0)); \
  (s3) = VEC_XOR((s3),(s1)); \
  (s1) = VEC_XOR((s1),(s2)); \
  (s0) = VEC_XOR((s0),(s3)); \
  (s2) = VEC_XOR((s2),t_); \
  (s3) = VEC_ROTL45((s3)); \
  (outv) = r_; \
} while (0)

HIDDEN void fill_fast_avx2(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  xo256 *st = &rng->state.xo;
  VEC_T s0 = VEC_LOAD(&st->s0[0]);
  VEC_T s1 = VEC_LOAD(&st->s1[0]);
  VEC_T s2 = VEC_LOAD(&st->s2[0]);
  VEC_T s3 = VEC_LOAD(&st->s3[0]);
#if defined(RANDOMPACK_TEST_HOOKS)
  avx2_used++;
#endif
  for (size_t i = 0; i < len; i += 4) {
    VEC_T r;
    FAST_STEP_VEC(s0, s1, s2, s3, r);
    VEC_STORE(out + i, r);
  }
  VEC_STORE(&st->s0[0], s0);
  VEC_STORE(&st->s1[0], s1);
  VEC_STORE(&st->s2[0], s2);
  VEC_STORE(&st->s3[0], s3);
}

HIDDEN void rand_dble_avx2(double x[], size_t len, randompack_rng *rng) {
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
      __m256i expo = _mm256_set1_epi64x(0x3ff0000000000000LL);
      __m256d one = _mm256_set1_pd(1);
      for (; j + 3 < take; j += 4) {
        __m256i r = _mm256_loadu_si256((const __m256i_u *)(u64 + j));
        r = _mm256_srli_epi64(r, 12);
        r = _mm256_or_si256(r, expo);
        __m256d d = _mm256_castsi256_pd(r);
        d = _mm256_sub_pd(d, one);
        _mm256_storeu_pd(x + i + j, d);
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

HIDDEN void rand_float_avx2(float x[], size_t len, randompack_rng *rng) {
  int w = enter_u32_mode(rng);
  size_t i = 0;
  while (i < len) {
    fill_if_empty32(rng, &w);
    size_t remain = len - i;
    size_t avail = 2*BUFSIZE - (size_t)w;
    size_t take = remain < avail ? remain : avail;
    uint32_t *u32 = rng->buf.u32 + (size_t)w;
    if (rng->usefullmantissa) {
      size_t j = 0;
      __m256 scale = _mm256_set1_ps(0x1.0p-24f);
      for (; j + 7 < take; j += 8) {
        __m256i r = _mm256_loadu_si256((const __m256i_u *)(u32 + j));
        r = _mm256_srli_epi32(r, 8);
        __m256 d = _mm256_cvtepi32_ps(r);
        d = _mm256_mul_ps(d, scale);
        _mm256_storeu_ps(x + i + j, d);
      }
      for (; j < take; j++) {
        x[i + j] = U32_TO_FLOAT(u32[j]);
      }
    }
    else {
      size_t j = 0;
      __m256 scale = _mm256_set1_ps(0x1.0p-23f);
      for (; j + 7 < take; j += 8) {
        __m256i r = _mm256_loadu_si256((const __m256i_u *)(u32 + j));
        r = _mm256_srli_epi32(r, 9);
        __m256 d = _mm256_cvtepi32_ps(r);
        d = _mm256_mul_ps(d, scale);
        _mm256_storeu_ps(x + i + j, d);
      }
      for (; j < take; j++) {
        x[i + j] = (float)(u32[j] >> 9)*0x1.0p-23f;
      }
    }
    w += (int)take;
    i += take;
  }
  exit_u32_mode(rng, w);
}
