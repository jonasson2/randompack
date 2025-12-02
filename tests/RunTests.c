#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "printX.h"
#include "Tests.h"
#include "xCheck.h"

static int NTOTAL = 0, NFAIL = 0;
static const char *headr_fmt = "%-20s %8s %8s\n";
static const char *table_fmt = "%-20s %8d %8d\n";
int TESTVERBOSITY = 0; // External

static void print_help(void) {
  puts("Usage: RunTests [options]\n"
       "Options:\n"
       "  -h    Show this help message\n"
       "  -v    Verbose tests\n"
       "  -vv   More verbosity\n"
       "  -vvv  Even moren verbosity\n"
       );
}

// -v    Summary
// -vv  Also printX

static void vprint(const char *fmt, ...) {
  if (TESTVERBOSITY < 1) return;
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  fflush(stdout);
}

static void option_error(const char *fmt, ...) {
  // Prints option error message formatted by fmt and exits
  char msg_opt[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(msg_opt, sizeof(msg_opt), fmt, args);
  va_end(args);
  fprintf(stderr, "%s\n", msg_opt);
  exit(1);
}

static void run_test(const char *name, void (*fn)(void)) {
  int ntotal, nfail;
  xCheckInit(name);
  fn();
  ntotal = xCheckNTotal();
  nfail  = xCheckNFailures();
  NTOTAL += ntotal;
  NFAIL  += nfail;
  vprint(table_fmt, name, ntotal - nfail, nfail);
}

static void run_bool_test(const char *name, bool (*fn)(void)) {
  int ntotal, nfail;
  xCheckInit(name);
  bool ok = fn();
  xCheck(ok);
  ntotal = xCheckNTotal();
  nfail  = xCheckNFailures();
  NTOTAL += ntotal;
  NFAIL  += nfail;
  vprint(table_fmt, name, ntotal - nfail, nfail);
}

int main(int argc, char **argv) {
  char optstring[10] = ":vh", c;
  while ((c = getopt(argc, argv, optstring)) != -1) {
    switch (c) {
      case 'h': print_help(); return 0;
      case 'v': TESTVERBOSITY++; break;
      case '?': option_error("Unknown option -%c", optopt);
    }
  }
  if (optind < argc) option_error("Unexpected argument %s", argv[optind]);
  if (TESTVERBOSITY <= 1) printOff();
  vprint("\n");
  vprint(headr_fmt, "TEST OF", "PASSED", "FAILED");
  run_test("RandomNumbers",      TestRandomNumbers);
  run_test("RandomNumbers_mvn",  TestRandomMvn);
  vprint(table_fmt, "TOTAL", NTOTAL - NFAIL, NFAIL);
  return (NFAIL > 0);
}
