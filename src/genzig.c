// gen_ziggurat_derived.c
#include <stdio.h>
#include <stdint.h>
#include "ziggurat_constants.h"

static void print_ke2(void) {
  int n = (int)(sizeof(ke_double) / sizeof(ke_double[0]));
  printf("static const uint64_t ke2[%d] = {\n", n);
  for (int i = 0; i < n; i++) {
    uint64_t v = ke_double[i] << 1;
    printf("  0x%016llXULL", (unsigned long long)v);
    if (i + 1 < n) printf(",");
    if ((i & 3) == 3) printf("\n");
  }
  if ((n & 3) != 0) printf("\n");
  printf("};\n\n");
}

static void print_weh(void) {
  int n = (int)(sizeof(we_double) / sizeof(we_double[0]));
  printf("static const double weh[%d] = {\n", n);
  for (int i = 0; i < n; i++) {
    printf("  %.17e", 0.5 * we_double[i]);
    if (i + 1 < n) printf(",");
    if ((i & 3) == 3) printf("\n");
  }
  if ((n & 3) != 0) printf("\n");
  printf("};\n\n");
}

static void print_ke2f(void) {
  int n = (int)(sizeof(ke_float) / sizeof(ke_float[0]));
  printf("static const uint32_t ke2f[%d] = {\n", n);
  for (int i = 0; i < n; i++) {
    uint32_t v = ke_float[i] << 1;
    printf("  0x%08XU", v);
    if (i + 1 < n) printf(",");
    if ((i & 7) == 7) printf("\n");
  }
  if ((n & 7) != 0) printf("\n");
  printf("};\n\n");
}

static void print_wehf(void) {
  int n = (int)(sizeof(we_float) / sizeof(we_float[0]));
  printf("static const float wehf[%d] = {\n", n);
  for (int i = 0; i < n; i++) {
    printf("  %.9eF", 0.5f * we_float[i]);
    if (i + 1 < n) printf(",");
    if ((i & 3) == 3) printf("\n");
  }
  if ((n & 3) != 0) printf("\n");
  printf("};\n");
}

int main(void) {
  print_ke2();
  print_weh();
  print_ke2f();
  print_wehf();
  return 0;
}
