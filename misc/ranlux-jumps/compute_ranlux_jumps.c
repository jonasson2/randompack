// -*- C -*-

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "randompack_internal.h"
#define fill_ranluxpp compute_ranluxpp_fill
#include "ranluxpp.inc"
#undef fill_ranluxpp

static int jump_p[] = {16, 20, 32, 64, 96, 128, 192};

static void step_state(uint64_t x[9]);

static void set_identity(uint64_t x[9]) {
  memset(x, 0, 9*sizeof(*x));
  x[0] = 1;
}

static void set_sample_state(uint64_t x[9]) {
  set_identity(x);
  for (int i = 0; i < 37; i++) step_state(x);
}

static void set_base_multiplier(uint64_t a[9]) {
  copy64(a, ranlux_a, 9);
}

static void mulmod9x9(uint64_t out[9], uint64_t x[9], uint64_t y[9]) {
  uint64_t z[18];
  mul9x9(z, x, y);
  mod9x9(out, z);
}

static void square9(uint64_t x[9]) {
  uint64_t y[9];
  copy64(y, x, 9);
  mulmod9x9(x, y, y);
}

static void step_state(uint64_t x[9]) {
  uint64_t z[18];
  mul9x9(z, x, ranlux_a);
  mod9x9(x, z);
}

static bool same9(uint64_t x[9], uint64_t y[9]) {
  for (int i = 0; i < 9; i++) if (x[i] != y[i]) return false;
  return true;
}

static void print9(char *name, uint64_t x[9]) {
  printf("static const uint64_t %s[9] = {\n", name);
  for (int i = 0; i < 9; i++) {
    printf("  0x%016llxULL%s\n", (unsigned long long)x[i], i < 8 ? "," : "");
  }
  printf("};\n\n");
}

static void report_mismatch(char *label, int p, uint64_t got[9], uint64_t want[9]) {
  fprintf(stderr, "check failed for p=%d on %s\n", p, label);
  for (int i = 0; i < 9; i++) {
    if (got[i] != want[i]) {
      fprintf(stderr, "word %d: got  0x%016llx\n", i, (unsigned long long)got[i]);
      fprintf(stderr, "word %d: want 0x%016llx\n", i, (unsigned long long)want[i]);
      return;
    }
  }
}

static bool verify_jump_on_state(int p, uint64_t jump[9], uint64_t start[9], char *label) {
  uint64_t direct[9], jumped[9];
  uint64_t steps;
  uint64_t t0, t1;
  if (p < 0 || p > 32) {
    fprintf(stderr, "direct check only implemented for p <= 32\n");
    return false;
  }
  steps = 1ULL << p;
  copy64(direct, start, 9);
  copy64(jumped, start, 9);
  t0 = clock_nsec();
  for (uint64_t i = 0; i < steps; i++) {
    step_state(direct);
  }
  t1 = clock_nsec();
  mulmod9x9(jumped, jump, jumped);
  if (!same9(direct, jumped)) {
    report_mismatch(label, p, jumped, direct);
    return false;
  }
  fprintf(stderr, "verified p=%d on %s in %.3f s\n",
    p, label, (double)(t1 - t0)*1e-9);
  return true;
}

static bool parse_check_arg(char *arg, bool *check16, bool *check20) {
  if (strcmp(arg, "16") == 0) {
    *check16 = true;
    return true;
  }
  if (strcmp(arg, "20") == 0) {
    *check20 = true;
    return true;
  }
  if (strcmp(arg, "all") == 0) {
    *check16 = true;
    *check20 = true;
    return true;
  }
  return false;
}

int main(int argc, char **argv) {
  // Precompute ranlux jump constants. 
  uint64_t jumps[LEN(jump_p)][9];
  uint64_t cur[9], x0[9], x1[9];
  bool check16 = false, check20 = false;
  int pcur = 0;
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--check") == 0) {
      if (i + 1 >= argc || !parse_check_arg(argv[i + 1], &check16, &check20)) {
        fprintf(stderr, "usage: %s [-c 16|20|all]\n", argv[0]);
        return 1;
      }
      i++;
    }
    else {
      fprintf(stderr, "usage: %s [-c 16|20|all]\n", argv[0]);
      return 1;
    }
  }
  set_base_multiplier(cur);
  for (int i = 0; i < LEN(jump_p); i++) {
    while (pcur < jump_p[i]) {
      square9(cur);
      pcur++;
    }
    copy64(jumps[i], cur, 9);
  }
  if (check16 || check20) {
    set_identity(x0);
    set_sample_state(x1);
    for (int i = 0; i < LEN(jump_p); i++) {
      if ((jump_p[i] == 16 && check16) || (jump_p[i] == 20 && check20)) {
        if (!verify_jump_on_state(jump_p[i], jumps[i], x0, "identity")) return 1;
        if (!verify_jump_on_state(jump_p[i], jumps[i], x1, "sample")) return 1;
      }
    }
  }
  for (int i = 0; i < LEN(jump_p); i++) {
    char name[32];
    STRSETF(name, "jump%d", jump_p[i]);
    print9(name, jumps[i]);
  }
  return 0;
}
