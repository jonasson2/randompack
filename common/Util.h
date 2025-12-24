// Common utilities for tests and benchmarks
#ifndef COMMON_UTIL_H
#define COMMON_UTIL_H

#include <stdio.h>
#include <stdlib.h>

#include "randompack_config.h"

#define ASSERT(e) do { \
  if (!(e)) { \
    fprintf(stderr, "ASSERT failed %s:%d: %s\n", __FILE__, __LINE__, #e); \
    abort(); \
  } \
} while (0)

#define TEST_ALLOC(p, n) ASSERT(ALLOC((p), (n)))

double get_time(void);
void warmup_cpu(int n);

#endif
