#include "Util.h"

#include <stdint.h>
#include <time.h>

#include "randompack_config.h"
#include "randompack.h"

#ifdef _WIN32
  #include <windows.h>
#endif

// Get current time in seconds (high resolution, monotonic)
double get_time(void) {
#ifdef _WIN32
  LARGE_INTEGER freq, counter;
  QueryPerformanceFrequency(&freq);
  QueryPerformanceCounter(&counter);
  return (double)counter.QuadPart/freq.QuadPart;
#elif defined(_POSIX_MONOTONIC_CLOCK)
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec + ts.tv_nsec/1e9;
#else
  // Fallback to standard C (lower resolution)
  return (double)clock()/CLOCKS_PER_SEC;
#endif
}

// Warm up the CPU by drawing n million uint64 randoms with xoshiro256++
void warmup_cpu(int n) {
  #define NBUF 1000
  uint64_t buf[NBUF];
  randompack_rng *rng = randompack_create("xoshiro256++");
  ASSERT(rng);
  ASSERT(randompack_seed(1, 0, 0, rng));
  int reps = max(1, n*1000000/NBUF);
  for (int i = 0; i < reps; i++) {
    ASSERT(randompack_uint64(buf, NBUF, 0, rng));
  }
  randompack_free(rng);
}
