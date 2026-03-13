// -*- C -*-
#include <assert.h>

#include "jumps.txt"

static inline void ranlux_jump(uint64_t s[9], int p) {
  const uint64_t *jump;
  uint64_t z[18];
  switch (p) {
  case 32:  jump = jump32;  break;
  case 64:  jump = jump64;  break;
  case 96:  jump = jump96;  break;
  case 128: jump = jump128; break;
  case 192: jump = jump192; break;
  default:  assert(0); }
  mul9x9(z, jump, s);
  mod9x9(s, z);
}
