#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "error.h"

static FILE *error_stream = 0;
static bool error_stream_explicit = false;
static char error_prefix[64] = "";

void varmapack_set_errstream(FILE *stream) {
  error_stream = stream;
  error_stream_explicit = true;
}

void varmapack_set_errprefix(char *prefix) {
  STRSET(error_prefix, prefix);
}

static void error_init(void) {
  if (!error_stream_explicit && error_stream == 0) {
    error_stream = stderr;
  }
}

void error_report(const char *file, int line, const char *fmt, ...) {
  error_init();
  if (error_stream == 0) return;
  va_list ap;
  va_start(ap, fmt);
  fprintf(error_stream, "error in %s:%d: ", basename_only(file), line);
  if (error_prefix[0])
    fprintf(error_stream, "%s: ", error_prefix);
  vfprintf(error_stream, fmt, ap);
  fprintf(error_stream, "\n");
  va_end(ap);
}
