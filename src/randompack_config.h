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

#define TOLOWER(c) (((c) >= 'A' && (c) <= 'Z') ? ((c)-'A'+'a') : (c))
#define STRSET(dst, src) snprintf((dst), sizeof(dst), "%s", (src) ? (src) : "")
#define STRSETF(dst, fmt, ...) snprintf((dst), sizeof(dst), (fmt), __VA_ARGS__)
#define LEN(a) ((int)(sizeof(a) / sizeof((a)[0])))
#define CLEAR(dst) memset((dst), 0, sizeof(dst))
#define ALLOC(ptr, count) (((ptr) = calloc((size_t)(count), sizeof *(ptr))) != 0)
#define FREE(p)  do { free(p); (p) = 0; } while (0)

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
