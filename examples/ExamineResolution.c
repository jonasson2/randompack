// -*- C -*-
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "randompack.h"

static const uint32_t GRID_SIZE = 1u << 24;
static const float EPSF = 0x1.0p-24f;

static const int DECIMALS = 8;
static const int ROWS = 6;
static const int SWEEPS = 20;

static const int LABELW = 8;

static void label_low(char out[LABELW + 1], int k) {
  if (k == 0) snprintf(out, (size_t)(LABELW + 1), "0");
  else if (k == 1) snprintf(out, (size_t)(LABELW + 1), "eps");
  else snprintf(out, (size_t)(LABELW + 1), "%deps", k);
}

static void label_high(char out[LABELW + 1], int k) {
  snprintf(out, (size_t)(LABELW + 1), "1-%deps", k);
}

int main(void) {
  float *x = 0;
  float *y = 0;
  randompack_rng *rng = 0;
  uint64_t cnt_low[ROWS + 1];
  uint64_t cnt_high[ROWS + 1];
  for (int k = 0; k <= ROWS; k++) cnt_low[k] = 0;
  for (int k = 0; k <= ROWS; k++) cnt_high[k] = 0;
  x = (float *)malloc((size_t)GRID_SIZE*sizeof(float));
  y = (float *)malloc((size_t)GRID_SIZE*sizeof(float));
  if (!x || !y) {
    fprintf(stderr, "ExamineResolution: allocation failed\n");
    free(x);
    free(y);
    return 1;
  }
  rng = randompack_create(0);
  if (!rng) {
    fprintf(stderr, "ExamineResolution: rng allocation failed\n");
    free(x);
    free(y);
    return 1;
  }
  printf("ExamineResolution (float U01)\n");
  printf("eps         = 2^-24 = %.3g\n", (double)EPSF);
  double grid_m = (double)GRID_SIZE*1e-6;
  double draws_m = (double)GRID_SIZE*(double)SWEEPS*1e-6;
  printf("grid size   = %.2f M\n", grid_m);
  printf("sweeps      = %d\n", SWEEPS);
  printf("total draws = %.2f M\n\n", draws_m);
  printf("%-*s  %*s  %7s\n", LABELW, "x", DECIMALS + 2, "decimal", "count");
  for (int i = 0; i < LABELW + 2 + (DECIMALS + 2) + 2 + 7; i++) putchar('-');
  putchar('\n');
  uint64_t grid_misses = 0;
  const float lo_cut = (float)ROWS*EPSF;
  const float hi_cut = 1.0f - (float)ROWS*EPSF;
  for (int sweep = 0; sweep < SWEEPS; sweep++) {
    if (!randompack_u01f(x, (size_t)GRID_SIZE, rng)) {
      fprintf(stderr, "ExamineResolution: randompack_u01f failed\n");
      randompack_free(rng);
      free(x);
      free(y);
      return 1;
    }
    uint32_t ksel = 0;
    for (uint32_t i = 0; i < GRID_SIZE; i++) {
      float u = x[i];
      if (u <= lo_cut || u >= hi_cut) y[ksel++] = u;
    }
    for (uint32_t j = 0; j < ksel; j++) {
      float u = y[j];
      uint32_t idx = (uint32_t)((double)u*(double)GRID_SIZE + 0.5);
      if (idx >= GRID_SIZE) { grid_misses++; continue; }
      double u2 = (double)idx*(double)EPSF;
      if ((double)u != u2) { grid_misses++; continue; }
      if (idx <= (uint32_t)ROWS) cnt_low[idx]++;
      else if (idx >= GRID_SIZE - (uint32_t)ROWS) {
        uint32_t d = GRID_SIZE - idx;
        if (d <= (uint32_t)ROWS) cnt_high[d]++;
      }
    }
  }
  uint64_t total = 0;
  for (int k = 0; k <= ROWS; k++) {
    char lab[LABELW + 1];
    label_low(lab, k);
    float u = (float)k*EPSF;
    printf("%-*s  %.*f  %7llu\n", LABELW, lab, DECIMALS, (double)u,
           (unsigned long long)cnt_low[k]);
    total += cnt_low[k];
  }
  for (int k = ROWS; k >= 1; k--) {
    char lab[LABELW + 1];
    label_high(lab, k);
    float u = 1.0f - (float)k*EPSF;
    printf("%-*s  %.*f  %7llu\n", LABELW, lab, DECIMALS, (double)u,
           (unsigned long long)cnt_high[k]);
    total += cnt_high[k];
  }
  printf("\n%-*s  %*s  %7llu\n", LABELW, "total", DECIMALS + 2, "",
         (unsigned long long)total);
  printf("grid_misses = %llu\n", (unsigned long long)grid_misses);
  randompack_free(rng);
  free(x);
  free(y);
  return 0;
}
