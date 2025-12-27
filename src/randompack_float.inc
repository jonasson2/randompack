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
