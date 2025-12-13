// TestSeedSeq.cpp
// Thorough testing that the C version of seed_seq_fe128 is a "verbatim" translation of
// Melissa O'Neill's original version, by running both on a fairly comprehensive set of
// input sequences, both "random" and edge cases and checking that the outputs of both
// functions agree bit-for-bit in each case. This is a ChatGPT-generated program.

#include <cstdint>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <iostream>

// --- Access hack: expose mixer_ for testing only ---
#define private public
#include "randutils.hpp"   // Melissa's header as-is
#undef private

extern "C" {
#include "seed_seq_fe128.inc"
}

static void CheckSeedSeqFe128Pool(std::vector<uint32_t> seeds) {
  // C++ reference
  randutils::seed_seq_fe128 ss_cpp(seeds.begin(), seeds.end());

  // C translation
  seed_seq_fe128 ss_c;
  uint32_t *begin = seeds.empty() ? nullptr : seeds.data();
  uint32_t *end   = seeds.empty() ? nullptr : seeds.data() + seeds.size();
  seed_seq_fe128_seed(&ss_c, begin, seeds.size());

  // Compare the 4-word entropy pool bit-for-bit
  for (int i=0; i<4; i++) {
    uint32_t a = ss_cpp.mixer_[i];
    uint32_t b = ss_c.mixer[i];
    if (a != b) {
      std::ostringstream msg;
      msg << "seed_seq_fe128 pool mismatch at i=" << i
          << " (cpp=0x" << std::hex << a
          << ", c=0x"   << std::hex << b << ")";
      throw std::runtime_error(msg.str());
    }
  }
}

static void RunSeedSeqFe128PoolTests() {
  // ---- A. Boundary-by-M tests ----
  CheckSeedSeqFe128Pool({});                 // M=0
  CheckSeedSeqFe128Pool({0});                // M=1
  CheckSeedSeqFe128Pool({0,1});              // M=2
  CheckSeedSeqFe128Pool({0,1,2});            // M=3
  CheckSeedSeqFe128Pool({0,1,2,3});          // M=4
  CheckSeedSeqFe128Pool({0,1,2,3,4});        // M=5

  std::vector<uint32_t> v20;
  for (uint32_t i=0; i<20; i++)
    v20.push_back(i);
  CheckSeedSeqFe128Pool(v20);                // M=20

  // ---- B. Value edge cases ----
  CheckSeedSeqFe128Pool({0,0,0,0});
  CheckSeedSeqFe128Pool({0xffffffffu,0xffffffffu,0xffffffffu,0xffffffffu});
  CheckSeedSeqFe128Pool({1,1,1,1});
  CheckSeedSeqFe128Pool({0x80000000u,0x80000000u,0x80000000u,0x80000000u});
  CheckSeedSeqFe128Pool({0x00000001u,0x00010000u,0x01000000u,0x80000000u});
  CheckSeedSeqFe128Pool({0xffffffffu,0,0xffffffffu,0,0xffffffffu});

  // ---- C. Random fuzz tests ----
  std::mt19937_64 rng(123456);  // fixed seed for reproducibility
  for (int t=0; t<1000; t++) {
    int M = rng() % 32;
    std::vector<uint32_t> seeds;
    seeds.reserve(M);
    for (int i=0; i<M; i++)
      seeds.push_back(uint32_t(rng()));
    CheckSeedSeqFe128Pool(seeds);
  }
}

int main() {
  try {
    RunSeedSeqFe128PoolTests();
  } catch (const std::exception& e) {
    std::cerr << "TEST FAILED: " << e.what() << "\n";
    return 1;
  }
  std::cout << "All seed_seq_fe128 pool tests passed.\n";
  return 0;
}
