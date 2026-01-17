// -*- C -*-

bool randompack_u01f(float x[], size_t len, randompack_rng *rng) {
  if (!rng) return false;
  if (!x)
    rng->last_error = "invalid arguments to randompack_u01f";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  rand_float(x, len, rng);
  return true;
}

bool randompack_uniff(float x[], size_t len, float a, float b, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || !(a < b)) {
    rng->last_error = "invalid arguments to randompack_uniff";
    return false;
  }
  rng->last_error = 0;
  rand_float(x, len, rng); // x in [0,1)
  float w = b - a;
  for (size_t i = 0; i < len; i++) {
#if defined(__clang__)
    x[i] = fminf(a + w*x[i], nextafter(b, a));
#else
    float v = a + w*x[i];
    x[i] = v < b ? v : nextafter(b, a);
#endif
  }
  return true;
}

bool randompack_normf(float x[], size_t len, randompack_rng *rng) {
  if (!rng) return false;
  if (!x)
    rng->last_error = "invalid arguments to randompack_normf";
  else
    rng->last_error = 0;
  if (rng->last_error) return false;
  rand_normf(x, len, rng);
  return true;
}

bool randompack_expf(float x[], size_t len, float scale, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || scale <= 0) {
    rng->last_error = "invalid arguments to randompack_expf";
    return false;
  }
  rng->last_error = 0;
  rand_expf(x, len, rng);
  if (scale != 1.0f) for (size_t i = 0; i < len; i++) x[i] *= scale;
  return true;
}

bool randompack_normalf(float x[], size_t len, float mu, float sigma,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || sigma <= 0) {
    rng->last_error = "invalid arguments to randompack_normalf";
    return false;
  }
  rng->last_error = 0;
  rand_normalf(x, len, mu, sigma, rng);
  // if (mu != 0.0f || sigma != 1.0f)
  //   for (size_t i = 0; i < len; i++) x[i] = mu + sigma*x[i];
  return true;
}

bool randompack_lognormalf(float x[], size_t len, float mu, float sigma,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || sigma <= 0) {
    rng->last_error = "invalid arguments to randompack_lognormalf";
    return false;
  }
  rng->last_error = 0;
  rand_normf(x, len, rng);
  for (size_t i = 0; i < len; i++) x[i] = expf(mu + sigma*x[i]);
  return true;
}

bool randompack_gumbelf(float x[], size_t len, float mu, float beta,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || beta <= 0.0f) {
    rng->last_error = "invalid arguments to randompack_gumbelf";
    return false;
  }
  rng->last_error = 0;
  rand_float(x, len, rng); // x in [0,1)
  for (size_t i = 0; i < len; i++)
    x[i] = mu - beta*logf(-logf(x[i]));
  return true;
}

bool randompack_paretof(float x[], size_t len, float xm, float alpha,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || xm <= 0 || alpha <= 0) {
    rng->last_error = "invalid arguments to randompack_paretof";
    return false;
  }
  rng->last_error = 0;
  rand_expf(x, len, rng);
  for (size_t i = 0; i < len; i++)
    x[i] = xm*expf(x[i]/alpha);
  return true;
}

bool randompack_gammaf(float x[], size_t len, float shape, float scale,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || shape <= 0 || scale <= 0) {
    rng->last_error = "invalid arguments to randompack_gammaf";
    return false;
  }
  rng->last_error = 0;
  fill_gammaf(x, len, shape, scale, rng);
  return true;
}

bool randompack_chi2f(float x[], size_t len, float nu, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || nu <= 0) {
    rng->last_error = "invalid arguments to randompack_chi2f";
    return false;
  }
  rng->last_error = 0;
  fill_gammaf(x, len, 0.5f*nu, 2.0f, rng);
  return true;
}

bool randompack_betaf(float x[], size_t len, float a, float b,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || a <= 0 || b <= 0) {
    rng->last_error = "invalid arguments to randompack_betaf";
    return false;
  }
  rng->last_error = 0;
  fill_betaf(x, len, a, b, rng);
  return true;
}

bool randompack_tf(float x[], size_t len, float nu, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || nu <= 0) {
    rng->last_error = "invalid arguments to randompack_tf";
    return false;
  }
  rng->last_error = 0;
  fill_tf(x, len, nu, rng);
  return true;
}

bool randompack_ff(float x[], size_t len, float nu1, float nu2,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || nu1 <= 0 || nu2 <= 0) {
    rng->last_error = "invalid arguments to randompack_ff";
    return false;
  }
  rng->last_error = 0;
  fill_ff(x, len, nu1, nu2, rng);
  return true;
}

bool randompack_weibullf(float x[], size_t len, float shape, float scale,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || shape <= 0 || scale <= 0) {
    rng->last_error = "invalid arguments to randompack_weibullf";
    return false;
  }
  rng->last_error = 0;
  rand_expf(x, len, rng);
  float inv_shape = 1.0f/shape;
  for (size_t i = 0; i < len; i++)
    x[i] = scale*powf(x[i], inv_shape);
  return true;
}
