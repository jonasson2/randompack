#if defined(__linux__) && !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#include "Util.h"

#include <stdint.h>
#include <time.h>
#if defined(__linux__)
#include <errno.h>
#include <sched.h>
#include <string.h>
#include <unistd.h>
#endif

#include "randompack_config.h"
#include "randompack.h"

#ifdef _WIN32
  #include <windows.h>
#endif

// -*- C -*-
#define _GNU_SOURCE
#include <errno.h>
#include <sched.h>
#include <stdio.h>
#include <string.h>

double get_time(void) { // seconds
  return 1e-9*(double)clock_nsec();
}  

// Warm up the CPU by drawing n million uint64 randoms with x256++
void warmup_cpu(int n) {
  #define NBUF 1000
  uint64_t buf[NBUF];
  randompack_rng *rng = randompack_create("x256++");
  ASSERT(rng);
  ASSERT(randompack_seed(1, 0, 0, rng));
  int reps = max(1, n*1000000/NBUF);
  for (int i = 0; i < reps; i++) {
    ASSERT(randompack_uint64(buf, NBUF, 0, rng));
  }
  randompack_free(rng);
}

void pin_to_cpu0(void) {
#if defined(__linux__)
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(0, &set);
  if (sched_setaffinity(0, (size_t)sizeof(set), &set) != 0)
    fprintf(stderr, "sched_setaffinity failed: %s\n", strerror(errno));
#endif
}
