// -*- C -*-
#ifndef MUL9X9_H
#define MUL9X9_H

#include <stdint.h>

void mul9x9(uint64_t *z, const uint64_t *restrict x);
void mod9x9(uint64_t x[9], uint64_t z[18]);

#endif
