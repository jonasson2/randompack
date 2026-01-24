#ifndef RANDOMPACK_CONFIG_H
#define RANDOMPACK_CONFIG_H

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#if defined(__SIZEOF_INT128__) && !defined(RANDOMPACK_PRETEND_NO128) \
    && !defined(RANDOMPACK_PRETEND_WINDOWS)
  #define HAVE128 1
  #define HAVE128MUL 1
  typedef __uint128_t uint128_t;
#elif (defined(_MSC_VER) && defined(_M_X64)) || defined(RANDOMPACK_PRETEND_WINDOWS)
  #define HAVE128 0
  #define HAVE128MUL 1
#else
  #define HAVE128 0
  #define HAVE128MUL 0
#endif

#endif
