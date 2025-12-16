// -*- C -*-
// Bit-exact check: compare randompack pcg64_dxsm, philox and xoshiro256{++,**}
// against NumPy, Random123, and Rust.

#include <stdint.h>
#include <stdbool.h>

#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"
#include "TestUtil.h"
#ifdef HAVE128

// -*- C -*-
// ChaCha20 test against RFC 8439 Section 2.3.2
// Authoritative source:
// https://www.rfc-editor.org/rfc/rfc8439.html

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "randompack.h"
#include "xCheck.h"

// Verbatim RFC material (bytes only, spaces only)

static const char *rfc8439_key_bytes =
"00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f "
"10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f";

static const char *rfc8439_nonce_bytes =
"00 00 00 09 00 00 00 4a 00 00 00 00";

static const char *rfc8439_serialized_block =
"10 f1 e7 e4 d1 3b 59 15 50 0f dd 1f a3 20 71 c4 "
"c7 d1 f4 c7 33 c0 68 03 04 22 aa 9a c3 d4 6c 4e "
"d2 82 64 46 07 9f aa 09 14 c2 d7 05 d9 8b 02 a2 "
"b5 12 9c d1 de 16 4e b9 cb d0 83 e8 a2 50 3c 4e";

// --- helpers ------------------------------------------------------------

static void parse_hex_bytes(const char *s, uint8_t *out, int n) {
  for (int i=0; i<n; i++) {
    unsigned int x;
    xCheck(sscanf(s, "%2x", &x) == 1);
    out[i] = (uint8_t)x;
    s += 2;
    if (*s == ' ') s++;
  }
}

// --- test ---------------------------------------------------------------

// static void TestChaCha20AgainstRFC8439(void) {
//   uint8_t key[32];
//   uint8_t nonce[12];
//   uint8_t expect[64];
//   uint8_t got[64];

//   parse_hex_bytes(rfc8439_key_bytes, key, 32);
//   parse_hex_bytes(rfc8439_nonce_bytes, nonce, 12);
//   parse_hex_bytes(rfc8439_serialized_block, expect, 64);

//   randompack_rng *rng = randompack_create("chacha20");
//   check_rng_clean(rng);

//   // RFC parameters:
//   // counter = 1, nonce = given, key = given
//   bool ok = randompack_set_chacha20(key, nonce, 1u, rng);
//   check_success(ok, rng);

//   ok = randompack_uint8(got, 64, 0, rng);
//   check_success(ok, rng);

//   xCheck(memcmp(got, expect, 64) == 0);

//   randompack_free(rng);
// }

static void TestAgainstNumpyPCG(void) {
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

  union {
    uint64_t u64[4];
    struct {
      uint128_t state;
      uint128_t inc;
    } u;
  } buf;

  buf.u.state = ((uint128_t)1 << 96) + 42;
  buf.u.inc   = (uint128_t)123;
  print64("u64[0]", buf.u64[0]);
  print64("u64[1]", buf.u64[1]);
  print64("u64[2]", buf.u64[2]);
  print64("u64[3]", buf.u64[3]);

  randompack_rng *rng = randompack_create("pcg64");
  check_rng_clean(rng);
  bool ok = randompack_set_state(buf.u64, 4, rng);
  check_success(ok, rng);
  uint64_t x[3];
  ok = randompack_uint64(x, 3, 0, rng);
  check_success(ok, rng);

  uint64_t py[3] = {
    2886615648589704684ull,
    10653487197848330975ull,
    15760175290616407522ull
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

static void TestPhiloxAgainstRandom123(void) {
  // Random123 version 1.14 was downloaded from github.com/DEShawResearch/random123,
  // and the following C program was then run to obtain randoms with the given state:
  //
  // #include "random123/include/Random123/philox.h"
  // #include <stdio.h>
  // int main(void) {
  //   philox4x64_key_t key = {{1, 2}};
  //   philox4x64_ctr_t ctr = {{1, 2, 3, 4}};
  //   philox4x64_ctr_t result = philox4x64(ctr, key);
  //   printf("draw0: %llu\n", result.v[0]);
  //   printf("draw1: %llu\n", result.v[1]);
  //   printf("draw2: %llu\n", result.v[2]);
  //   return 0;
  // }

  randompack_philox_key key = {1, 2};
  randompack_counter ctr = {1, 2, 3, 4};

  uint64_t x[3];
  bool ok = randompack_uint64_philox(x, 3, ctr, key);
  xCheck(ok);

  uint64_t r123[3] = {
	 15293350248826363405ull,
	 16459017380466521836ull,
	 14953335634064817228ull
  };

  xCheck(equal_vec64(x, r123, 3));

  printMsg("COMPARISON WITH ORIGINAL PHILOX:");
  printMsg("Random123 philox draws:");
  print64("r123[0]", r123[0]);
  print64("r123[1]", r123[1]);
  print64("r123[2]", r123[2]);
  printMsg("randompack philox draws:");
  print64("rp[0]", x[0]);
  print64("rp[1]", x[1]);
  print64("rp[2]", x[2]);
}

static void TestXoshiro256ppAgainstRust(void) {
  // Rust reference: Following program was used to obtain the state:
  // use rand::RngCore;
  // use rand::SeedableRng;
  // use rand_xoshiro::Xoshiro256PlusPlus;
  // fn main() {
  //     let state: [u64; 4] = [1234567890_123456789, 1, 2, 9876543210_987654321];
  //     let seed: [u8; 32] = unsafe { std::mem::transmute(state) };
  //     let mut rng = Xoshiro256PlusPlus::from_seed(seed);
  //     println!("{}", rng.next_u64());
  //     println!("{}", rng.next_u64());
  //     println!("{}", rng.next_u64());
  // }
  uint64_t state[4] = {
    1234567890123456789ull,
    1ull,
    2ull,
    9876543210987654321ull
  };
  randompack_rng *rng = randompack_create("xoshiro256++");
  check_rng_clean(rng);
  bool ok = randompack_set_state(state, 4, rng);
  check_success(ok, rng);
  uint64_t x[3];
  ok = randompack_uint64(x, 3, 0, rng);
  check_success(ok, rng);
  uint64_t rust[3] = {
    7481289576166103649ULL,
    14954838225025987065ULL,
    13925222450416462367ULL
  };
  xCheck(equal_vec64(x, rust, 3));
  printMsg("COMPARISON WITH RUST XOSHIRO256++:");
  printMsg("Rust xoshiro256++ draws:");
  print64("rust[0]", rust[0]);
  print64("rust[1]", rust[1]);
  print64("rust[2]", rust[2]);
  printMsg("randompack xoshiro256++ draws:");
  print64("rp[0]", x[0]);
  print64("rp[1]", x[1]);
  print64("rp[2]", x[2]);
  randompack_free(rng);
}

static void TestXoshiro256ssAgainstRust(void) {
  // use rand::RngCore;
  // use rand::SeedableRng;
  // use rand_xoshiro::Xoshiro256StarStar;
  // fn main() {
  //     let state: [u64; 4] = [111111_222222_333333, 444444_555555_666666,
  //                            777777_888888_999999, 888888_777777_666666];
  //     let seed: [u8; 32] = unsafe { std::mem::transmute(state) };    
  //     let mut rng = Xoshiro256StarStar::from_seed(seed);
  //     println!("{}", rng.next_u64());
  //     println!("{}", rng.next_u64());
  //     println!("{}", rng.next_u64());
  // }
  uint64_t state[4] = {
    111111222222333333ull, 444444555555666666ull,
    777777888888999999ull, 888888777777666666ull};
  randompack_rng *rng = randompack_create("xoshiro256**");
  check_rng_clean(rng);
  bool ok = randompack_set_state(state, 4, rng);
  check_success(ok, rng);
  uint64_t x[3];
  ok = randompack_uint64(x, 3, 0, rng);
  check_success(ok, rng);
  uint64_t rust[3] = {
    14349957828721873287ull,
    17468056612619362601ull,
    5100482715761765753ull
  };
  xCheck(equal_vec64(x, rust, 3));
  printMsg("COMPARISON WITH RUST XOSHIRO256**:");
  printMsg("Rust xoshiro256** draws:");
  print64("rust[0]", rust[0]);
  print64("rust[1]", rust[1]);
  print64("rust[2]", rust[2]);
  printMsg("randompack xoshiro256** draws:");
  print64("rp[0]", x[0]);
  print64("rp[1]", x[1]);
  print64("rp[2]", x[2]);
  randompack_free(rng);
}

void TestReference(void) {
  TestAgainstNumpyPCG();
  TestPhiloxAgainstRandom123();
  TestXoshiro256ppAgainstRust();
  TestXoshiro256ssAgainstRust();
  //TestChaCha20AgainstRFC8439();
}
