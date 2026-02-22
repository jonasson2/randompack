#ifndef RANDOMPACK_CONFIG_H
#define RANDOMPACK_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_MSC_VER)
  #include <intrin.h>
#endif

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "_Static_assert requires C11"
#endif

_Static_assert(sizeof(int) == 4, "randompack requires 32-bit int");
_Static_assert(sizeof(void*) == 8, "randompack requires 64-bit pointers");
_Static_assert(sizeof(long long) == 8, "randompack requires 64-bit long long");

#if defined(_MSC_VER)
  #define ALWAYS_INLINE static __forceinline
#elif defined(__clang__) || defined(__GNUC__)
  #define ALWAYS_INLINE static inline __attribute__((always_inline))
#else
  #define ALWAYS_INLINE static inline
#endif

#define TOLOWER(c) (((c) >= 'A' && (c) <= 'Z') ? ((c)-'A'+'a') : (c))
#define STRSET(dst, src) snprintf((dst), sizeof(dst), "%s", (src) ? (src) : "")
#define STRSETF(dst, fmt, ...) snprintf((dst), sizeof(dst), (fmt), __VA_ARGS__)
#define LEN(a) (int)((sizeof(a) / sizeof((a)[0])))
#define CLEAR(dst) memset((dst), 0, sizeof(dst))
#define ALLOC(ptr, count) (((ptr) = calloc((count), sizeof *(ptr))) != 0)
#define FREE(p)  do { free(p); (p) = 0; } while (0)
#define ROTL(x,k) (((x) << (k)) | ((x) >> (64 - (k))))
#define U32_TO_FLOAT(u) (((u) >> 8) * 0x1.0p-24f)
#define U64_TO_DOUBLE(u) (((u) >> 11) * 0x1.0p-53)
#define STRSETN(dst, maxlen, src) /* maxlen includes trailing 0 */   \
 do {                                                                \
   if ((dst) && (maxlen) > 0)                                        \
     snprintf((dst), (size_t)(maxlen), "%s", (src) ? (src) : "");    \
 } while (0)

static inline int min(int m, int n) { return m < n ? m : n; }
static inline int max(int m, int n) { return m > n ? m : n; }

static inline bool is_little_endian(void) {
  uint32_t one = 1;
  return *(uint8_t *)&one == 1;
}

static inline void copy16(void *dst, void *src, int n) { memcpy(dst, src, n*2); }
static inline void copy32(void *dst, void *src, int n) { memcpy(dst, src, n*4); }
static inline void copy64(void *dst, void *src, int n) { memcpy(dst, src, n*8); }

#ifndef BUFSIZE
#define BUFSIZE 128
#endif

_Static_assert(BUFSIZE % 8 == 0, "BUFSIZE must be a multiple of 8 (for chacha20)");
_Static_assert(BUFSIZE <= 512,
	       "BUFSIZE must be <= 512 to allow BUFSIZE arrays on the stack");
static inline uint64_t rand_splitmix64(uint64_t *x) {
  uint64_t z = (*x += 0x9E3779B97F4A7C15ULL);
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
  return z ^ (z >> 31);
}

static inline uint32_t mix32(uint32_t x) {
  x ^= x >> 16;
  x *= 0x7feb352dU;
  x ^= x >> 15;
  x *= 0x846ca68bU;
  x ^= x >> 16;
  return x;
}

#ifdef __unix__
  #include <unistd.h>
  #include <fcntl.h>
#elif defined(_WIN32)
  #include <windows.h>
#endif

#if defined(__SIZEOF_INT128__) && !defined(RANDOMPACK_PRETEND_NO128)
  #define HAVE128 1
  typedef __uint128_t uint128_t;
#else
  #define HAVE128 0
#endif

// 64x64 -> 128 (hi,lo). Prefer native, fall back to emulation.
#if HAVE128
#define MUL64_WIDE(a, b, hi, lo) do {                          \
  __uint128_t _p = ((__uint128_t)(a)) * ((__uint128_t)(b));    \
  (lo) = (uint64_t)_p;                                         \
  (hi) = (uint64_t)(_p >> 64);                                 \
} while (0)

#elif defined(_MSC_VER)
#define MUL64_WIDE(a, b, hi, lo) do {                          \
  (lo) = _umul128((a), (b), &(hi));                            \
} while (0)

#else
#define MUL64_WIDE(a, b, hi, lo) do {                          \
  uint64_t _a0 = (uint32_t)(a);                                \
  uint64_t _a1 = (a) >> 32;                                    \
  uint64_t _b0 = (uint32_t)(b);                                \
  uint64_t _b1 = (b) >> 32;                                    \
  uint64_t _p00 = _a0*_b0;                                     \
  uint64_t _p01 = _a0*_b1;                                     \
  uint64_t _p10 = _a1*_b0;                                     \
  uint64_t _p11 = _a1*_b1;                                     \
  uint64_t _mid = (_p00 >> 32) + (uint32_t)_p01                \
    + (uint32_t)_p10;                                          \
  (hi) = _p11 + (_p01 >> 32) + (_p10 >> 32) + (_mid >> 32);    \
  (lo) = (_mid << 32) | (uint32_t)_p00;                        \
} while (0)
#endif


#if defined(PRETEND_NOSIMD)
#define HAVE_NEON 0
#elif defined(__aarch65__) || defined(__ARM_NEON) || defined(__ARM_NEON__)
#define HAVE_NEON 1
#else
#define HAVE_NEON 0
#endif

#if defined(__GNUC__) || defined(__clang__)
#define LIKELY(x)   __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define LIKELY(x)   (x)
#define UNLIKELY(x) (x)
#endif

#define FAST_STEP_SCALAR(s0,s1,s2,s3,outv) do { \
  uint64_t r_; \
  uint64_t t_; \
  r_ = ROTL((s0) + (s3), 23) + (s0); \
  t_ = (s1) << 17; \
  (s2) ^= (s0); \
  (s3) ^= (s1); \
  (s1) ^= (s2); \
  (s0) ^= (s3); \
  (s2) ^= t_; \
  (s3) = ROTL((s3), 45); \
  (outv) = r_; \
} while (0)
#endif
