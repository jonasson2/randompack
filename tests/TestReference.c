// -*- C -*-
// Bit-exact check: compare randompack pcg64_dxsm against NumPy
//
// Reference state, increment, and draws obtained with the following Python program:
//
// import numpy as np
// bg = np.random.PCG64DXSM(seed=123)
// stdict = bg.state['state'];
// print("state:", stdict['state'])
// print("incre:", stdict['inc'])
// g = np.random.Generator(bg)
// x = g.integers(0, 2**64, size=3, dtype=np.uint64)
// print('draw0:', x[0])
// print('draw1:', x[1])
// print('draw2:', x[2])

#include <stdint.h>
#include <stdbool.h>

#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"
#include "TestUtil.h"
#ifdef HAVE128
static uint128_t parse_u128_dec(char *s) {
  uint128_t x = 0;
  for (int i = 0; s[i]; i++) x = x*10 + (uint128_t)(s[i] - '0');
  return x;
}

static void test_pcg64_dxsm_compare(void) {
  char *state_s = "160078363690744033601390112987726904141";
  char *inc_s   = "17686443629577124697969402389330893883";

  union {
    uint64_t u64[4];
    struct {
      uint128_t state;
      uint128_t inc;
    } u;
  } buf;

  buf.u.state = parse_u128_dec(state_s);
  buf.u.inc   = parse_u128_dec(inc_s);

  randompack_rng *rng = randompack_create("pcg64");
  check_rng_clean(rng);
  bool ok = randompack_set_state(buf.u64, 4, rng);
  check_success(ok, rng);
  uint64_t x[3];
  ok = randompack_uint64(x, 3, 0, rng);
  check_success(ok, rng);

  uint64_t py[3] = {
    12966207907714485482ULL,
    18052762201172092891ULL,
    3481888569593207593ULL
  };

  xCheck(equal_vec64(x, py, 3));

  printMsg("Python pcg64_dxsm draws:");
  print64("py[0]", py[0]);
  print64("py[1]", py[1]);
  print64("py[2]", py[2]);
  printMsg("randompack pcg64_dxsm draws:");
  print64("rp[0]", x[0]);
  print64("rp[1]", x[1]);
  print64("rp[2]", x[2]);

  randompack_free(rng);
}
#else
static void test_pcg64_dxsm_compare(void) {
}
#endif

void TestReference(void) {
  test_pcg64_dxsm_compare();
}
