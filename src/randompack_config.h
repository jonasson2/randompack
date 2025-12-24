#ifndef RANDOMPACK_CONFIG_H
#define RANDOMPACK_CONFIG_H

#include <limits.h>
#include <stdio.h>   // snprintf
#include <stdlib.h>  // calloc, free
#include <string.h>  // memset

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "_Static_assert requires C11"
#endif

_Static_assert(sizeof(int) == 4, "randompack requires 32-bit int");
_Static_assert(sizeof(void*) == 8, "randompack requires 64-bit pointers");
_Static_assert(sizeof(long long) == 8, "randompack requires 64-bit long long");

// Endianness check (compile-time)
#if defined(__BYTE_ORDER__) && defined(__ORDER_LITTLE_ENDIAN__) && \
  defined(__ORDER_BIG_ENDIAN__)
  #if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
    #error "randompack requires a little-endian target"
  #endif
#elif defined(_WIN32)
// Windows on supported targets is little-endian
#else
// Fallback: unknown at compile time; use runtime check.
  #define RANDOMPACK_NEED_RUNTIME_ENDIAN_CHECK 1
#endif

#define TOLOWER(c) (((c) >= 'A' && (c) <= 'Z') ? ((c)-'A'+'a') : (c))
#define STRSET(dst, src) snprintf((dst), sizeof(dst), "%s", (src) ? (src) : "")
#define STRSETF(dst, fmt, ...) snprintf((dst), sizeof(dst), (fmt), __VA_ARGS__)
#define LEN(a) (int)((sizeof(a) / sizeof((a)[0])))
#define CLEAR(dst) memset((dst), 0, sizeof(dst))
#define ALLOC(ptr, count) (((ptr) = calloc((count), sizeof *(ptr))) != 0)
#define FREE(p)  do { free(p); (p) = 0; } while (0)
static inline int min(int m, int n) { return m < n ? m : n; }
static inline int max(int m, int n) { return m > n ? m : n; }

static inline void copy16(void *dst, void *src, int n) { memcpy(dst, src, (size_t)n*2); }
static inline void copy32(void *dst, void *src, int n) { memcpy(dst, src, (size_t)n*4); }
static inline void copy64(void *dst, void *src, int n) { memcpy(dst, src, (size_t)n*8); }
#ifndef BUFSIZE
#define BUFSIZE 256
#endif
_Static_assert(BUFSIZE % 8 == 0, "BUFSIZE must be a multiple of 8 uint64 words (64 bytes)");

static inline uint64_t rand_splitmix64(uint64_t *x) {
  uint64_t z = (*x += 0x9E3779B97F4A7C15ULL);
  z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
  z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
  return z ^ (z >> 31);
}

static int mersenne8 = 2147483647;  // 2^31-1

#ifdef __unix__
  #include <unistd.h>
  #include <fcntl.h>
#elif defined(_WIN32)
  #include <windows.h>
#endif

#if defined(__SIZEOF_INT128__) && !defined(_MSC_VER)
  #define HAVE128 1
  typedef __uint128_t uint128_t;
#endif

#if HAVE128 || (defined(_MSC_VER) && defined(_M_X64))
  #define HAVE128MUL 1
#endif

#endif
