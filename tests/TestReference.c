// -*- C -*-
// Bit-exact check: compare randompack pcg64, philox and x256{++,**}
// against NumPy, Random123, and Rust.

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "randompack.h"
#include "randompack_config.h"
#include "printX.h"
#include "xCheck.h"
#include "test_util.h"

// ChaCha20 test against RFC 8439 Section 2.3.2
// Authoritative source:
// https://www.rfc-editor.org/rfc/rfc8439.html

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
  for (int i = 0; i < n; i++) {
    unsigned int x;
    xCheck(sscanf(s, "%2x", &x) == 1);
    out[i] = x;
    s += 2;
    if (*s == ' ') s++;
  }
}

static uint32_t pack32le(uint8_t *b) {
  uint32_t w0 = b[0];
  uint32_t w1 = b[1] << 8;
  uint32_t w2 = b[2] << 16;
  uint32_t w3 = (uint32_t)b[3] << 24;
  return w0 | w1 | w2 | w3;
}

static void build_chacha_state(uint64_t *state, uint8_t *key, uint8_t *nonce,
                               uint32_t counter) {
  uint32_t w[12];
  for (int i = 0; i < 8; i++) w[i] = pack32le(key + 4*i);
  for (int i = 0; i < 3; i++) w[8 + i] = pack32le(nonce + 4*i);
  w[11] = counter;
  for (int i = 0; i < 6; i++) {
    state[i] = w[2*i];
    state[i] |= (uint64_t)w[2*i + 1] << 32;
  }
}

static void TestChaCha20AgainstRFC8439(void) {
  uint8_t key[32];
  uint8_t nonce[12];
  uint8_t expect[64];
  uint8_t got[64];
  uint64_t state[6];

  parse_hex_bytes(rfc8439_key_bytes, key, 32);
  parse_hex_bytes(rfc8439_nonce_bytes, nonce, 12);
  parse_hex_bytes(rfc8439_serialized_block, expect, 64);

  randompack_rng *rng = randompack_create("chacha20");
  check_rng_clean(rng);

  // RFC parameters: counter = 1, nonce = given, key = given
  build_chacha_state(state, key, nonce, 1u);
  bool ok = randompack_set_state(state, LEN(state), rng);
  check_success(ok, rng);

  ok = randompack_uint8(got, 64, 0, rng);
  check_success(ok, rng);

  xCheck(memcmp(got, expect, 64) == 0);

  randompack_free(rng);
}

#if HAVE128

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

  CHECK_EQUALV(x, py, 3);

  printMsg("Python pcg64 (dxsm) draws:");
  print64("py[0]", py[0]);
  print64("py[1]", py[1]);
  print64("py[2]", py[2]);
  printMsg("randompack pcg64 draws:");
  print64("rp[0]", x[0]);
  print64("rp[1]", x[1]);
  print64("rp[2]", x[2]);

  randompack_free(rng);
}
#else
static void TestAgainstNumpyPCG(void) {
}
#endif

static void TestPhiloxAgainstRandom123(void) {
  uint64_t x[5];
  randompack_rng *rng = randompack_create("philox");
  uint64_t state[6] = {1, 2, 3, 4, 1, 2};
  bool ok = randompack_set_state(state, 6, rng);
  check_success(ok, rng);
  ok = randompack_uint64(x, 5, 0, rng);
  check_success(ok, rng);
  uint64_t r123[] = {
    15293350248826363405ull,
    16459017380466521836ull,
    14953335634064817228ull,
    8379624740030528727ull,
    14229447488281503856ull
  };
  CHECK_EQUALV(x, r123, 5);
  randompack_free(rng);
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
  //     rng.jump();
  //     println!("{}", rng.next_u64());
  //     rng.long_jump();
  //     println!("{}", rng.next_u64());
  // }
  uint64_t state[4] = {
    1234567890123456789ull,
    1ull,
    2ull,
    9876543210987654321ull
  };
  randompack_rng *rng = randompack_create("x256++");
  check_rng_clean(rng);
  bool ok = randompack_set_state(state, 4, rng);
  check_success(ok, rng);
  uint64_t x[5];
  ok = randompack_uint64(x, 3, 0, rng);
  check_success(ok, rng);
  uint64_t state_after3[4] = {
    2528806104241426626ULL, 6362598626627523027ULL,
    17214963705533504631ULL, 14411775491225371596ULL
  };
  ok = randompack_set_state(state_after3, 4, rng);
  check_success(ok, rng);
  ok = randompack_jump(128, rng);
  check_success(ok, rng);
  ok = randompack_uint64(x + 3, 1, 0, rng);
  check_success(ok, rng);
  ok = randompack_set_state(state, 4, rng);
  check_success(ok, rng);
  ok = randompack_jump(192, rng);
  check_success(ok, rng);
  ok = randompack_uint64(x + 4, 1, 0, rng);
  check_success(ok, rng);
  uint64_t rust[5] = {
    7481289576166103649ULL,
    14954838225025987065ULL,
    13925222450416462367ULL,
    16338411700179016235ULL,
    14555093047527680046ULL
  };
  CHECK_EQUALV(x, rust, 5);
  printMsg("COMPARISON WITH RUST X256++:");
  printMsg("Rust x256++ draws:");
  print64("rust[0]", rust[0]);
  print64("rust[1]", rust[1]);
  print64("rust[2]", rust[2]);
  print64("rust[3]", rust[3]);
  print64("rust[4]", rust[4]);
  printMsg("randompack x256++ draws:");
  print64("rp[0]", x[0]);
  print64("rp[1]", x[1]);
  print64("rp[2]", x[2]);
  print64("rp[3]", x[3]);
  print64("rp[4]", x[4]);
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
  //     rng.jump();
  //     println!("{}", rng.next_u64());
  //     rng.long_jump();
  //     println!("{}", rng.next_u64());
  // }
  uint64_t state[4] = {
    111111222222333333ull, 444444555555666666ull,
    777777888888999999ull, 888888777777666666ull};
  randompack_rng *rng = randompack_create("x256**");
  check_rng_clean(rng);
  bool ok = randompack_set_state(state, 4, rng);
  check_success(ok, rng);
  uint64_t x[5];
  ok = randompack_uint64(x, 3, 0, rng);
  check_success(ok, rng);
  uint64_t state_after3[4] = {
    17259611468371763932ULL, 7463711936217166741ULL,
    14826803636522241130ULL, 2576392830761277348ULL
  };
  ok = randompack_set_state(state_after3, 4, rng);
  check_success(ok, rng);
  ok = randompack_jump(128, rng);
  check_success(ok, rng);
  ok = randompack_uint64(x + 3, 1, 0, rng);
  check_success(ok, rng);
  ok = randompack_set_state(state, 4, rng);
  check_success(ok, rng);
  ok = randompack_jump(192, rng);
  check_success(ok, rng);
  ok = randompack_uint64(x + 4, 1, 0, rng);
  check_success(ok, rng);
  uint64_t rust[5] = {
    14349957828721873287ull,
    17468056612619362601ull,
    5100482715761765753ull,
    11684350292475811581ull,
    11305255822621342761ull
  };
  CHECK_EQUALV(x, rust, 5);
  printMsg("COMPARISON WITH RUST X256**:");
  printMsg("Rust x256** draws:");
  print64("rust[0]", rust[0]);
  print64("rust[1]", rust[1]);
  print64("rust[2]", rust[2]);
  print64("rust[3]", rust[3]);
  print64("rust[4]", rust[4]);
  printMsg("randompack x256** draws:");
  print64("rp[0]", x[0]);
  print64("rp[1]", x[1]);
  print64("rp[2]", x[2]);
  print64("rp[3]", x[3]);
  print64("rp[4]", x[4]);
  randompack_free(rng);
}

static void TestRanluxppAgainstJirka(void) {
#if BUFSIZE % 9 != 0
  xCheck(true);
  return;
#endif
  // Reference values can be reproduced with misc/ranlux-work/ref_ranlux_portable.c.
  // That program sets x = {1..9}, advances 100 states in ranluxpp-portable,
  // xors the 900 output words, and records the last word.
  // Jirka's implementation has been independently verified against Hahnfeld-Moneta
  // (see misc/ranlux-work/extract_hahnmon_lcg.cpp for verification).
  uint64_t xorsum = 0xfe4bac4d5cedb127ULL, lastval = 0xee4ef07d92e6614dULL;
  uint64_t init[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  uint64_t x[900];
  randompack_rng *rng = randompack_create("ranlux++");
  check_rng_clean(rng);
  bool ok = randompack_set_state(init, 9, rng);
  check_success(ok, rng);
  ok = randompack_uint64(x, 900, 0, rng);
  check_success(ok, rng);
  uint64_t acc = 0;
  for (int i = 0; i < 900; i++) acc ^= x[i];
  xCheck(acc == xorsum);
  xCheck(x[899] == lastval);
  randompack_free(rng);
}


void TestReference(void) {
  TestChaCha20AgainstRFC8439();
  if (!is_little_endian()) return;
  TestAgainstNumpyPCG();
  TestPhiloxAgainstRandom123();
  TestXoshiro256ppAgainstRust();
  TestXoshiro256ssAgainstRust();
  TestRanluxppAgainstJirka();
}

void TestReferencex(char *engine) {
  char *e = engine;
  if (!strcmp(e, "chacha20")) {
    TestChaCha20AgainstRFC8439();
    return;
  }
  if (!is_little_endian()) return;
  if (!strcmp(e, "pcg64")) {
    TestAgainstNumpyPCG();
  }
  else if (!strcmp(e, "philox")) {
    TestPhiloxAgainstRandom123();
  }
  else if (!strcmp(e, "x256++")) {
    TestXoshiro256ppAgainstRust();
  }
  else if (!strcmp(e, "x256**")) {
    TestXoshiro256ssAgainstRust();
  }
  else if (!strcmp(e, "ranlux++")) {
    TestRanluxppAgainstJirka();
  }
}
