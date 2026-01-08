// -*- C -*-
// Tests for randompack_seed: determinism, and spawn_key separation

#include <stdbool.h>
#include <stdint.h>
#include "randompack.h"
#include "TestUtil.h"
#include "xCheck.h"

static void draw_stream(char *engine, uint64_t *x, int n,
                        int seed, uint32_t *spawn_key, int nkey) {
  randompack_rng *rng = randompack_create(engine);
  ASSERT(rng);
  check_rng_clean(rng);
  bool ok = randompack_seed(seed, spawn_key, nkey, rng);
  check_success(ok, rng);
  ok = randompack_uint64(x, n, 0, rng);
  check_success(ok, rng);
  randompack_free(rng);
}

static void draw_stream_reseed_same_rng(char *engine, uint64_t *x, uint64_t *y, int n,
                                        int seed, uint32_t *spawn_key, int nkey) {
  randompack_rng *rng = randompack_create(engine);
  ASSERT(rng);
  check_rng_clean(rng);
  bool ok = randompack_seed(seed, spawn_key, nkey, rng);
  check_success(ok, rng);
  ok = randompack_uint64(x, n, 0, rng);
  check_success(ok, rng);

  // Reseed the same RNG and confirm we restart the stream.
  ok = randompack_seed(seed, spawn_key, nkey, rng);
  check_success(ok, rng);
  ok = randompack_uint64(y, n, 0, rng);
  check_success(ok, rng);
  randompack_free(rng);
}

static void test_seed_determinism(void) {
  // Determinism with a spawn key is tested for the seed-sequence engines below.
  char *engines_seq[] = {
    "x128+",
    "squares",
    "x256**",
    "x256++",
    "pcg64",
    "cwg128",
    "philox",
    "chacha20",
  };

  enum { LEN_STREAM = 4 };
  uint32_t key[] = {7u, 11u, 13u};

  // Seed-sequence determinism (same key, same seed)
  for (int i = 0; i < LEN(engines_seq); i++) {
    uint64_t x[LEN_STREAM], y[LEN_STREAM], z[LEN_STREAM], r[LEN_STREAM];
    draw_stream(engines_seq[i], x, LEN_STREAM, 42, key, LEN(key));
    draw_stream(engines_seq[i], y, LEN_STREAM, 42, key, LEN(key));
    xCheck(equal_vec64(x, y, LEN_STREAM));
    draw_stream(engines_seq[i], z, LEN_STREAM, 43, key, LEN(key));
    xCheck(everywhere_different(x, z, LEN_STREAM));
    draw_stream_reseed_same_rng(engines_seq[i], x, r, LEN_STREAM, 42, key, LEN(key));
    xCheck(equal_vec64(x, r, LEN_STREAM));
  }
}

static void test_spawn_key_separation(void) {
  // These tests target the "structural collision" class: [] vs [0], etc.
  char *engines[] = {
    "x128+",
    "squares",
    "x256**",
    "x256++",
    "pcg64",
    "cwg128",
    "philox",
    "chacha20",
  };

  enum { LEN_STREAM = 4, NENGINES = LEN(engines) };
  uint32_t k0[] = {0u};
  uint32_t k321[] = {3u, 2u, 1u};
  uint32_t k3210[] = {3u, 2u, 1u, 0u};
  for (int i = 0; i < NENGINES; i++) {
    for (int seed = 0; seed < 4; seed++) {
      uint64_t a[LEN_STREAM], b[LEN_STREAM];

      // seed, key=[]  vs  seed, key=[0]
      draw_stream(engines[i], a, LEN_STREAM, seed, 0, 0);
      draw_stream(engines[i], b, LEN_STREAM, seed, k0, 1);
      xCheck(everywhere_different(a, b, LEN_STREAM));

      // seed, key=[3,2,1]  vs  seed, key=[3,2,1,0]
      draw_stream(engines[i], a, LEN_STREAM, seed, k321, 3);
      draw_stream(engines[i], b, LEN_STREAM, seed, k3210, 4);
      xCheck(everywhere_different(a, b, LEN_STREAM));
    }
  }
}

void TestSeed(void) {
  test_seed_determinism();
  test_spawn_key_separation();
}
