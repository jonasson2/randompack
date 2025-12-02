// Safeguarded allocate that includes sizeof
#ifndef ALLOCATE_H
#define ALLOCATE_H
#include <assert.h>
#include <stdlib.h>

#define freem(p) free(p)
#define allocate(p, len) {                      \
   assert((len) >= 0);                          \
   (p) = calloc((size_t)(len), sizeof(*(p)));   \
   assert(((len) == 0) || (p));                 \
 }
#define checkStatus()
#define checkLeak()
#endif
