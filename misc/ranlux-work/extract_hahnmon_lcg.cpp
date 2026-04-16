// Extract LCG state from hahnmon RanluxppEngine to compare with portable
#include <cstdio>
#include <cstdint>
#include <cstring>

// We need to access private members of RanluxppEngine
// So we'll create a local copy with public access for testing

namespace {
  static constexpr int kStateElements = 9;
  static constexpr int kStateElementBits = 64;
  static constexpr int kMaxPos = kStateElements * kStateElementBits;

  const uint64_t kA_2048[] = {
    0xed7faa90747aaad9, 0x4cec2c78af55c101, 0xe64dcb31c48228ec,
    0x6d8a15a13bee7cb0, 0x20b2ca60cb78c509, 0x256c3d3c662ea36c,
    0xff74e54107684ed2, 0x492edfcc0cc8e753, 0xb48c187cf5b22097,
  };
}

#include "ranluxpp/helpers.h"
#include "ranluxpp/ranlux_lcg.h"
#include "ranluxpp/mulmod.h"

// Minimal reimplementation to access state
class RanluxppEngineTest {
private:
  uint64_t fState[kStateElements];
  unsigned fCarry;
  int fPosition = 0;

  void Advance() {
    uint64_t lcg[kStateElements];
    to_lcg(fState, fCarry, lcg);
    mulmod(kA_2048, lcg);
    to_ranlux(lcg, fState, fCarry);
    fPosition = 0;
  }

public:
  RanluxppEngineTest(uint64_t seed) {
    SetSeed(seed);
  }

  void SetSeed(uint64_t s) {
    uint64_t lcg[kStateElements];
    lcg[0] = 1;
    for (int i = 1; i < kStateElements; i++) {
      lcg[i] = 0;
    }

    uint64_t a_seed[kStateElements];
    // Skip 2 ** 96 states.
    static constexpr uint64_t TwoTo48 = uint64_t(1) << 48;
    powermod(kA_2048, a_seed, TwoTo48);
    powermod(a_seed, a_seed, TwoTo48);
    // Skip another s states.
    powermod(a_seed, a_seed, s);
    mulmod(a_seed, lcg);

    to_ranlux(lcg, fState, fCarry);
    fPosition = 0;
  }

  void GetLCGState(uint64_t *lcg) {
    to_lcg(fState, fCarry, lcg);
  }

  void AdvanceState() {
    Advance();
  }
};

int main(void) {
  RanluxppEngineTest rng(1);

  uint64_t buf[9*100];

  for (int i = 0; i < 100; i++) {
    uint64_t lcg[9];
    rng.GetLCGState(lcg);
    memcpy(buf + 9*i, lcg, 9*sizeof(uint64_t));
    rng.AdvanceState();
  }

  // Write to stdout
  fwrite(buf, 1, 7200, stdout);

  return 0;
}
