// -*- mode: c; -*-
// RNG engine implementations
#include "xorshift128p.inc"
#include "xoshiro256pp.inc"
#include "xoshiro256ss.inc"
#include "chachacha.inc"
#include "csprng.inc"
#include "squares64.inc"

#ifdef HAVE128
#include "pcg64_dxsm.inc"
#include "cwg128.inc"
#endif

#ifdef HAVE128MUL
#include "philox4x64.inc"
#endif
