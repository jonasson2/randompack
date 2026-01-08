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
    printf(fmt, base, line, func, ctx, message);
  }
  else {
    char fmt[] = "%s:%d (%s): test failed, %s is false";
    printf(fmt, base, line, func, message);
  }
  printf("\n");
  fflush(stdout);
  NTOTAL += 1;
  NFAIL += 1;
}

void xCheckFunc2(char *message, const char *file, int line, const char *func, const char
					  *msg1, const char *msg2) {
  char buf[256];
  if (msg1 && msg2) snprintf(buf, sizeof(buf), "%s:%s", msg1, msg2);
  else if (msg1) snprintf(buf, sizeof(buf), "%s", msg1);
  else if (msg2) snprintf(buf, sizeof(buf), "%s", msg2);
  else buf[0] = 0;
  xCheckFunc(message, file, line, func, buf[0] ? buf : 0);
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
