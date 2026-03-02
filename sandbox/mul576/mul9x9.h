// -*- C -*-
#ifndef MUL9X9_H
#define MUL9X9_H

#include <stdint.h>

void mul9x9(uint64_t *out, const uint64_t *restrict b);
void mod9x9(uint64_t *x);

#endif
