// -*- mode: c; -*-
// RNG engine implementations
#include "xorshift128p.inc"
#include "xoshiro256pp.inc"
#include "xoshiro256ss.inc"
#include "xoroshiro128pp.inc"
#include "x256ppsimd.inc"
#include "chachacha.inc"
#include "csprng.inc"
#include "squares64.inc"
#include "sfc64.inc"
#include "x256pp_jump.inc"

#if HAVE128
#include "pcg64_dxsm.inc"
#include "cwg128.inc"
#endif

#if HAVE128MUL
#include "philox4x64.inc"
#endif

