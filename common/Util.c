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

double get_time(void) { // seconds
  return 1e-9*(double)clock_nsec();
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
