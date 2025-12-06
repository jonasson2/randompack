#include <stdio.h>
#include <string.h>
#include "xCheck.h"

static int NTOTAL = 0;
static int NFAIL = 0;

static const char *basename_path(const char *file) {
  const char *slash = strrchr(file, '/');
  const char *bslash = strrchr(file, '\\');
  const char *base = file;
  if (slash && (!bslash || slash > bslash)) {
    base = slash + 1;
  }
  else if (bslash) {
    base = bslash + 1;
  }
  return base;
}

void xCheckFunc(char *message, const char *file, int line, const char *func, const char
                *ctx) {
  const char *base = basename_path(file);
  if (ctx) {
    char fmt[] = "%s:%d (%s, %s): test failed, %s is false";
    fprintf(stderr, fmt, base, line, func, ctx, message);
  }
  else {
    char fmt[] = "%s:%d (%s): test failed, %s is false";
    fprintf(stderr, fmt, base, line, func, message);
  }
  fprintf(stderr, "\n");
  fflush(stderr);
  NTOTAL += 1;
  NFAIL += 1;
}

void xCheckOK(void) {
  NTOTAL += 1;
}

void xCheckInit(void) { // reset counters
  NTOTAL = 0;
  NFAIL = 0;
}

int xCheckNFailures(void) {
  return NFAIL;
}

int xCheckNTotal(void) {
  return NTOTAL;
}
