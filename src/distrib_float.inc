// -*- mode: c; -*-
#if defined(USE_ACCEL_VV)
void vvexpf(float *y, const float *x, const int *n);
void vvlogf(float *y, const float *x, const int *n);
#endif

static void rand_lognormalf(float x[], size_t len, float mu, float sigma,
  randompack_rng *rng) {
  rand_normf(x, len, rng); // N(0,1)
  if (mu != 0 || sigma != 1) shift_scale_float_inplace(x, len, mu, sigma, rng);
  expf_inplace(x, len, rng);
}

ALWAYS_INLINE void gen_gamma_shape1f(float *x, size_t len, float shape,
  float scale, randompack_rng *rng) {
  float d = shape - 1.0f/3.0f, dscale = d*scale;
  float c = 1.0f/sqrtf(9.0f*d), v3, z, v;
  rand_normf(x, len, rng);
  int w = enter_u32_mode(rng);
  for (size_t i = 0; i < len; i++) {
    for (;;) {
      z = x[i];
      v = 1.0f + c*z;
      if (v > 0.0f) {
        float u, z2 = z*z, z4 = z2*z2, logu, logv3;
        v3 = v*v*v;
        u = draw_u01f(rng, &w);
        if (u < 1.0f - 0.0331f*z4) {
          x[i] = dscale*v3;
          break;
        }
        if (!rng->bitexact) {
          logu = logf(u);
          logv3 = logf(v3);
        }
        else {
          logu = openlibm_logf(u);
          logv3 = openlibm_logf(v3);
        }
        if (logu < 0.5f*z2 + d*(1.0f - v3 + logv3)) {
          x[i] = dscale*v3;
          break;
        }
      }
      x[i] = draw_normf(rng, &w);
    }
  }
  exit_u32_mode(rng, w);
}

ALWAYS_INLINE void gen_gammaf(float *x, size_t len, float shape, float scale,
  randompack_rng *rng) {
  if (shape >= 1.0f) {
    gen_gamma_shape1f(x, len, shape, scale, rng);
  }
  else {
    float inv_shape = 1.0f/shape;
    float t[512];
    gen_gamma_shape1f(x, len, shape + 1.0f, scale, rng);
    size_t i = 0;
    while (i < len) {
      size_t n = len - i < 256 ? len - i : 256;
      rand_float(t, n, rng);
      for (size_t j = 0; j < n; j++) {
        float y = rng->bitexact ? openlibm_log1pf(-t[j]) : log1pf(-t[j]);
        t[j] = rng->bitexact ? openlibm_expf(inv_shape*y) : expf(inv_shape*y);
      }
      for (size_t j = 0; j < n; j++) x[i + j] *= t[j];
      i += n;
    }
  }
}

ALWAYS_INLINE void gen_betaf(float *x, size_t len, float a, float b,
  randompack_rng *rng) {
  gen_gammaf(x, len, a, 1.0f, rng);
  float gb[64];
  size_t i = 0;
  while (i < len) {
    int n = mini(64, (int)(len - i));
    gen_gammaf(gb, n, b, 1.0f, rng);
    float hi = 1.0f - FLT_EPSILON; // restrict draws to (0, 1):
    for (int j = 0; j < n; j++) {
      float ga = x[i + j];
      float gbj = gb[j];
      float y = ga / (ga + gbj);
      if (y < FLT_MIN) y = FLT_MIN;
      if (y > hi) y = hi;
      x[i + j] = y;
    }
    i += n;
  }
}

ALWAYS_INLINE void gen_tf(float *x, size_t len, float nu, randompack_rng *rng) {
  float u[64];
  rand_normf(x, len, rng);
  size_t i = 0;
  while (i < len) {
    int n = mini(64, (int)(len - i));
    gen_gammaf(u, n, 0.5f*nu, 2.0f, rng);
    for (int j = 0; j < n; j++)
      x[i + j] /= sqrtf(u[j]/nu);
    i += n;
  }
}

ALWAYS_INLINE void gen_ff(float *x, size_t len, float nu1, float nu2,
  randompack_rng *rng) {
  float x2[64];
  gen_gammaf(x, len, nu1/2.0f, 1.0f, rng);
  size_t i = 0;
  while (i < len) {
    int n = mini(64, (int)(len - i));
    gen_gammaf(x2, n, nu2/2.0f, 1.0f, rng);
    for (int j = 0; j < n; j++)
      x[i + j] = (x[i + j]*nu2)/(x2[j]*nu1);
    i += n;
  }
}

ALWAYS_INLINE void gen_chi2f(float *x, size_t len, float nu,
  randompack_rng *rng) {
  gen_gammaf(x, len, 0.5f*nu, 2, rng);
}

ALWAYS_INLINE void gen_gumbelf(float *x, size_t len, float mu, float beta,
  randompack_rng *rng) {
  rand_float(x, len, rng); // x in [0,1)
  logf_inplace(x, len, rng);
  for (size_t i = 0; i < len; i++) x[i] = -x[i];
  logf_inplace(x, len, rng);
  shift_scale_float_inplace(x, len, mu, -beta, rng);
}

ALWAYS_INLINE void gen_paretof(float *x, size_t len, float xm, float alpha,
  randompack_rng *rng) {
  rand_expf(x, len, rng);
  for (size_t i = 0; i < len; i++) x[i] = x[i]/alpha;
  expf_inplace(x, len, rng);
  scale_float_inplace(x, len, xm, rng);
}

ALWAYS_INLINE void gen_weibullf(float *x, size_t len, float shape,
  float scale, randompack_rng *rng) {
  rand_expf(x, len, rng);
  if (shape == 1.0f) {
    if (scale != 1.0f) scale_float_inplace(x, len, scale, rng);
    return;
  }
  if (shape == 2.0f) {
    for (size_t i = 0; i < len; i++) x[i] = sqrtf(x[i]);
    if (scale != 1.0f) scale_float_inplace(x, len, scale, rng);
    return;
  }
  float inv_shape = 1.0f/shape;
  logf_inplace(x, len, rng);
  scale_float_inplace(x, len, inv_shape, rng);
  expf_inplace(x, len, rng);
  if (scale != 1.0f) scale_float_inplace(x, len, scale, rng);
}

ALWAYS_INLINE void gen_skew_normalf(float *x, size_t len, float mu,
  float sigma, float alpha, randompack_rng *rng) {
  float denom = sqrtf(1.0f + alpha*alpha);
  float delta = alpha/denom;
  float scale = 1.0f/denom;
  rand_normf(x, len, rng);
  size_t i = 0;
  float u[64];
  while (i < len) {
    int n = mini(64, (int)(len - i));
    rand_normf(u, n, rng);
    for (int j = 0; j < n; j++) {
      float v = delta*fabsf(u[j]) + scale*x[i + j];
      x[i + j] = mu + sigma*v;
    }
    i += n;
  }
}
