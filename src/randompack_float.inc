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
  rand_expf(x, len, scale, rng);
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
  for (size_t i = 0; i < len; i++) x[i] = fminf(b, a + w*x[i]);
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
  rand_normf(x, len, rng);
  if (mu != 0.0f || sigma != 1.0f)
    for (size_t i = 0; i < len; i++) x[i] = mu + sigma*x[i];
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
  for (size_t i = 0; i < len; i++) {
    float u = x[i];
    while (u <= 0.0f) u = (float)draw_u01(rng);
    x[i] = mu - beta*logf(-logf(u));
  }
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
  for (size_t i = 0; i < len; i++) {
    float u = (float)draw_u01(rng);
    while (u <= 0.0f) u = (float)draw_u01(rng);
    x[i] = xm*expf(-logf(u)/alpha);
  }
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
  double sh = shape;
  if (scale == 1.0f) {
    for (size_t i = 0; i < len; i++) x[i] = (float)gamma_shape(sh, rng);
  }
  else {
    double sc = scale;
    for (size_t i = 0; i < len; i++)
      x[i] = (float)(gamma_shape(sh, rng)*sc);
  }
  return true;
}

bool randompack_chi2f(float x[], size_t len, float nu, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || nu <= 0) {
    rng->last_error = "invalid arguments to randompack_chi2f";
    return false;
  }
  rng->last_error = 0;
  double nud = nu;
  for (size_t i = 0; i < len; i++) {
    double g = gamma_shape(0.5*nud, rng);
    x[i] = (float)(g*2.0);
  }
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
  double ad = a;
  double bd = b;
  for (size_t i = 0; i < len; i++) {
    for (;;) {
      double ga = gamma_shape(ad, rng);
      double gb = gamma_shape(bd, rng);
      double s = ga + gb;
      if (s > 0) {
        float y = (float)(ga/s);
        if (y > 0.0f && y < 1.0f) {
          x[i] = y;
          break;
        }
      }
    }
  }
  return true;
}

bool randompack_tf(float x[], size_t len, float nu, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || nu <= 0) {
    rng->last_error = "invalid arguments to randompack_tf";
    return false;
  }
  rng->last_error = 0;
  double nud = nu;
  for (size_t i = 0; i < len; i++) {
    double z, u;
    rand_norm(&z, 1, rng);
    u = gamma_shape(0.5*nud, rng)*2.0;
    x[i] = (float)(z/sqrt(u/nud));
  }
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
  double nu1d = nu1;
  double nu2d = nu2;
  for (size_t i = 0; i < len; i++) {
    double x1 = gamma_shape(nu1d/2.0, rng);
    double x2 = gamma_shape(nu2d/2.0, rng);
    x[i] = (float)((x1*nu2d)/(x2*nu1d));
  }
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
  rand_float(x, len, rng); // x in [0,1)
  float inv_shape = 1.0f/shape;
  for (size_t i = 0; i < len; i++) {
    float u = x[i];
    x[i] = scale*powf(-log1pf(-u), inv_shape);
  }
  return true;
}
