// -*- mode: c; -*-
// RNG engine implementations
#include "xorshift128p.inc"
#include "xoshiro256pp.inc"
#include "xoshiro256ss.inc"
#include "chachacha.inc"
#include "counter_based.inc"
#include "csprng.inc"

#ifdef HAVE128
#include "pcg64_dxsm.inc"
#endif

#ifdef HAVE128MUL
#include "philox_rng.inc"
#endif
