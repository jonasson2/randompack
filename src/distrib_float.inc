// -*- mode: c; -*-

ALWAYS_INLINE static void fill_gamma_shape1f(float *x, size_t len, float shape,
															randompack_rng *rng) {
  float d = shape - 1.0f/3.0f;
  float c = 1.0f/sqrtf(9.0f*d);
  rand_normf(x, len, rng);
  for (size_t i = 0; i < len; i++) {
    for (;;) {
      float z = x[i];
      float v = 1.0f + c*z;
      if (v > 0.0f) {
        float v3 = v*v*v;
        float u  = draw_u01f_fast(rng);
        float z2 = z*z;
        float z4 = z2*z2;
        if (u < 1.0f - 0.0331f*z4 ||
            logf(u) < 0.5f*z2 + d*(1.0f - v3 + logf(v3))) {
          x[i] = d*v3;
          break;
        }
      }
      x[i] = random_standard_normal_f(rng);
    }
  }
}

ALWAYS_INLINE static void fill_gammaf(float *x, size_t len, float shape, float scale,
  randompack_rng *rng) {
  enter_u32_mode(rng);
  if (shape >= 1.0f) {
    fill_gamma_shape1f(x, len, shape, rng);
  }
  else {
    fill_gamma_shape1f(x, len, shape + 1.0f, rng);
    for (size_t i = 0; i < len; i++) {
      float u = draw_u01f_fast(rng);
      x[i] *= expf(logf(u)/shape);
    }
  }
  if (scale != 1.0f) for (size_t i = 0; i < len; i++) x[i] *= scale;
  exit_u32_mode(rng);
}

ALWAYS_INLINE static void fill_betaf(float *x, size_t len, float a, float b,
  randompack_rng *rng) {
  enter_u32_mode(rng);
  fill_gammaf(x, len, a, 1.0f, rng);
  float gb[64];
  size_t i = 0;
  while (i < len) {
    int n = min(64, len - i);
    fill_gammaf(gb, n, b, 1.0f, rng);
    for (int j = 0; j < n; j++) {
      float y = x[i + j] / (x[i + j] + gb[j]);      
		x[i + j] = fmaxf(fminf(y, 1 - FLT_EPSILON), FLT_MIN);
    }
    i += n;
  }
  exit_u32_mode(rng);
}

ALWAYS_INLINE static void fill_tf(float *x, size_t len, float nu, randompack_rng *rng) {
  float u[64];
  rand_normf(x, len, rng);
  size_t i = 0;
  while (i < len) {
    int n = min(64, len - i);
    fill_gammaf(u, n, 0.5f*nu, 2.0f, rng);
    for (int j = 0; j < n; j++)
      x[i + j] /= sqrtf(u[j]/nu);
    i += n;
  }
}

ALWAYS_INLINE static void fill_ff(float *x, size_t len, float nu1, float nu2,
  randompack_rng *rng) {
  float x2[64];
  fill_gammaf(x, len, nu1/2.0f, 1.0f, rng);
  size_t i = 0;
  while (i < len) {
    int n = min(64, len - i);
    fill_gammaf(x2, n, nu2/2.0f, 1.0f, rng);
    for (int j = 0; j < n; j++)
      x[i + j] = (x[i + j]*nu2)/(x2[j]*nu1);
    i += n;
  }
}
