// -*- C -*-

#include <stdint.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include "randompack.h"
#include "randompack_internal.h"
#include "printX.h"
#include "openlibm.inc"
#include "log_exp_float.inc"
#include "buffer_draw.inc"
#include "scale_inplace_float.inc"
#include "ziggurat_const_float.h"
#include "zig_D_float.h"

#include "rand_float.inc"
#include "norm_exp_float.inc"
#include "distrib_float.inc"

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
  rand_float(x, len, rng);
  if (a == 0 && b == 1) return true;
#if defined(FP_FAST_FMA)
  float w = nextafterf(b - a, 0.0f);
#else
  float w = b - a;
#endif
  shift_scale_float_inplace(x, len, a, w, rng);
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

bool randompack_normalf(float x[], size_t len, float mu, float sigma,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || sigma <= 0) {
    rng->last_error = "invalid arguments to randompack_normalf";
    return false;
  }
  rng->last_error = 0;
  rand_normf(x, len, rng);
  if (mu != 0 || sigma != 1) shift_scale_float_inplace(x, len, mu, sigma, rng);
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
  if (scale != 1.0f) scale_float_inplace(x, len, scale, rng);
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
  rand_lognormalf(x, len, mu, sigma, rng);
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
  gen_gammaf(x, len, shape, scale, rng);
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
  gen_betaf(x, len, a, b, rng);
  return true;
}

bool randompack_chi2f(float x[], size_t len, float nu, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || nu <= 0) {
    rng->last_error = "invalid arguments to randompack_chi2f";
    return false;
  }
  rng->last_error = 0;
  gen_chi2f(x, len, nu, rng);
  return true;
}

bool randompack_tf(float x[], size_t len, float nu, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || nu <= 0) {
    rng->last_error = "invalid arguments to randompack_tf";
    return false;
  }
  rng->last_error = 0;
  gen_tf(x, len, nu, rng);
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
  gen_ff(x, len, nu1, nu2, rng);
  return true;
}

bool randompack_gumbelf(float x[], size_t len, float mu, float beta,
  randompack_rng *rng) {
  if (!rng) return false;
  if (!x || beta <= 0) {
    rng->last_error = "invalid arguments to randompack_gumbelf";
    return false;
  }
  rng->last_error = 0;
  gen_gumbelf(x, len, mu, beta, rng);
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
  gen_paretof(x, len, xm, alpha, rng);
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
  gen_weibullf(x, len, shape, scale, rng);
  return true;
}

bool randompack_skew_normalf(float x[], size_t len, float mu, float sigma,
  float alpha, randompack_rng *rng) {
  if (!rng) return false;
  if (!x || sigma <= 0) {
    rng->last_error = "invalid arguments to randompack_skew_normalf";
    return false;
  }
  rng->last_error = 0;
  gen_skew_normalf(x, len, mu, sigma, alpha, rng);
  return true;
}
