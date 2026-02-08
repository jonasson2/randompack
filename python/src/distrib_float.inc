// -*- mode: c; -*-
// ALWAYS_INLINE void rand_float(float x[], size_t len, randompack_rng *rng) {
//   int w = enter_u32_mode(rng);
//   size_t i = 0;
//   while (i < len) {
//     fill_if_empty32(rng, &w);
//     int take = min(len - i, 2*BUFSIZE - w);
//     for (int j = 0; j < take; j++) {
//       uint32_t v = rng->buf.u32[w + j];
//       x[i + j] = U32_TO_FLOAT(v);
//     }
//     w += take;
//     i += take;
//   }
//   exit_u32_mode(rng, w);
// }
#if defined(__APPLE__)
void vvexpf(float *y, const float *x, const int *n);
void vvlogf(float *y, const float *x, const int *n);
#endif

static void rand_lognormalf(float x[], size_t len, float mu, float sigma,
  randompack_rng *rng) {
  rand_normf(x, len, rng); // N(0,1)
#if defined(__APPLE__)
  for (size_t i = 0; i < len; ) {
    size_t chunk;
    if (len - i > (size_t)INT_MAX) {
      chunk = (size_t)INT_MAX;
    }
    else {
      chunk = len - i;
    }
    int n = (int)chunk;
    if (mu != 0 || sigma != 1)
      for (size_t j = 0; j < chunk; j++) x[i + j] = mu + sigma*x[i + j];
    vvexpf(x + i, x + i, &n);
    i += chunk;
  }
#else
  for (size_t i = 0; i < len; i++) x[i] = expf(mu + sigma*x[i]);
#endif
}

ALWAYS_INLINE void fill_gamma_shape1f(float *x, size_t len, float shape,
  randompack_rng *rng) {
  float d = shape - 1.0f/3.0f;
  float c = 1.0f/sqrtf(9.0f*d);
  rand_normf(x, len, rng);
  int w = enter_u32_mode(rng);
  for (size_t i = 0; i < len; i++) {
    for (;;) {
      float z = x[i];
      float v = 1.0f + c*z;
      if (v > 0.0f) {
        float v3 = v*v*v;
        float u  = draw_u01f(rng, &w);
        float z2 = z*z;
        float z4 = z2*z2;
        if (u < 1.0f - 0.0331f*z4 ||
            logf(u) < 0.5f*z2 + d*(1.0f - v3 + logf(v3))) {
          x[i] = d*v3;
          break;
        }
      }
      make_half_full32(rng, &w);
      x[i] = draw_normf(rng, &w);
    }
  }
  exit_u32_mode(rng, w);
}

ALWAYS_INLINE void fill_gammaf(float *x, size_t len, float shape, float scale,
  randompack_rng *rng) {
  if (shape >= 1.0f) {
    fill_gamma_shape1f(x, len, shape, rng);
  }
  else {
    fill_gamma_shape1f(x, len, shape + 1.0f, rng);
    int w = enter_u32_mode(rng);
    for (size_t i = 0; i < len; i++) {
      float u = draw_u01f(rng, &w);
      x[i] *= expf(logf(u)/shape);
    }
    exit_u32_mode(rng, w);
  }
  if (scale != 1.0f) for (size_t i = 0; i < len; i++) x[i] *= scale;
}

ALWAYS_INLINE void fill_betaf(float *x, size_t len, float a, float b,
  randompack_rng *rng) {
  fill_gammaf(x, len, a, 1.0f, rng);
  float gb[64];
  size_t i = 0;
  while (i < len) {
    int n = min(64, len - i);
    fill_gammaf(gb, n, b, 1.0f, rng);
    float hi = 1.0f - FLT_EPSILON; // restrict draws to (0, 1):
    for (int j = 0; j < n; j++) {
      float a = x[i + j];
      float b = gb[j];
      float y = a / (a + b);
      if (y < FLT_MIN) y = FLT_MIN;
      if (y > hi) y = hi;
      x[i + j] = y;
    }
    i += n;
  }
}

ALWAYS_INLINE void fill_tf(float *x, size_t len, float nu, randompack_rng *rng) {
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

ALWAYS_INLINE void fill_ff(float *x, size_t len, float nu1, float nu2,
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

ALWAYS_INLINE void fill_chi2f(float *x, size_t len, float nu,
  randompack_rng *rng) {
  fill_gammaf(x, len, 0.5f*nu, 2, rng);
}

ALWAYS_INLINE void fill_gumbelf(float *x, size_t len, float mu, float beta,
  randompack_rng *rng) {
  rand_float(x, len, rng); // x in [0,1)
#if defined(__APPLE__)
  for (size_t i = 0; i < len; ) {
    size_t chunk;
    if (len - i > (size_t)INT_MAX) {
      chunk = (size_t)INT_MAX;
    }
    else {
      chunk = len - i;
    }
    int n = (int)chunk;
    vvlogf(x + i, x + i, &n);
    for (size_t j = 0; j < chunk; j++) x[i + j] = -x[i + j];
    vvlogf(x + i, x + i, &n);
    for (size_t j = 0; j < chunk; j++)
      x[i + j] = mu - beta*x[i + j];
    i += chunk;
  }
#else
  for (size_t i = 0; i < len; i++)
    x[i] = mu - beta*logf(-logf(x[i]));
#endif
}

ALWAYS_INLINE void fill_paretof(float *x, size_t len, float xm, float alpha,
  randompack_rng *rng) {
  rand_expf(x, len, rng);
  for (size_t i = 0; i < len; i++)
    x[i] = xm*expf(x[i]/alpha);
}

ALWAYS_INLINE void fill_weibullf(float *x, size_t len, float shape,
  float scale, randompack_rng *rng) {
  rand_expf(x, len, rng);
  float inv_shape = 1.0f/shape;
  for (size_t i = 0; i < len; i++)
    x[i] = scale*powf(x[i], inv_shape);
}

ALWAYS_INLINE void fill_skew_normalf(float *x, size_t len, float mu,
  float sigma, float alpha, randompack_rng *rng) {
  float denom = sqrtf(1.0f + alpha*alpha);
  float delta = alpha/denom;
  float scale = 1.0f/denom;
  rand_normf(x, len, rng);
  size_t i = 0;
  float u[64];
  while (i < len) {
    int n = min(64, len - i);
    rand_normf(u, n, rng);
    for (int j = 0; j < n; j++) {
      float v = delta*fabsf(u[j]) + scale*x[i + j];
      x[i + j] = mu + sigma*v;
    }
    i += n;
  }
}
