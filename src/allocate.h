// Safeguarded allocate that includes sizeof
#ifndef ALLOCATE_H
#define ALLOCATE_H
#include <stdlib.h>
#include "error.h"
// Following are declared in stdlib.h and need (should) not be redeclared here
#define freem(p) free(p)
#define allocate(p, len) {                      \
   xAssert((len) >= 0);                         \
   (p) = calloc((size_t)(len), sizeof(*(p)));   \
   xAssert((p) != 0);                           \
 }
// Following are to be ignored:
#define checkStatus()
#define checkLeak()
#endif
