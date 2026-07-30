#ifndef RANDOMPACK_PYTHON_API_H
#define RANDOMPACK_PYTHON_API_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef RANDOMPACK_H
typedef struct randompack_rng randompack_rng;
#endif

#define RANDOMPACK_PYTHON_API_CAPSULE "randompack._core._C_API"
#define RANDOMPACK_PYTHON_API_VERSION 1

typedef struct {
  uint32_t abi_version;
  size_t struct_size;
  randompack_rng *(*create)(const char *engine);
  void (*free_rng)(randompack_rng *rng);
  char *(*last_error)(randompack_rng *rng);
  bool (*mvn)(char *transp, double mu[], double Sig[], int d, size_t len,
              double X[], int ldx, double L[], randompack_rng *rng);
  bool (*seed)(int seed, uint32_t *spawn_key, int n_key, randompack_rng *rng);
  bool (*u01)(double x[], size_t len, randompack_rng *rng);
} randompack_python_api;

#endif // RANDOMPACK_PYTHON_API_H
