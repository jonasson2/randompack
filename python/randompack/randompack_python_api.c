#include "randompack.h"
#include "randompack_python_api_internal.h"

static randompack_python_api python_api = {
  RANDOMPACK_PYTHON_API_VERSION,
  sizeof(randompack_python_api),
  randompack_create,
  randompack_free,
  randompack_last_error,
  randompack_mvn,
  randompack_seed,
  randompack_u01
};

HIDDEN randompack_python_api *randompack_python_api_table(void) {
  return &python_api;
}
