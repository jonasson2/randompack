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
static char *headr_fmt = "%-20s %8s %8s\n";
static char *table_fmt = "%-20s %8d %8d\n";
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

static void vprint(char *fmt, ...) {
  if (TESTVERBOSITY < 1) return;
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  fflush(stdout);
}

static void option_error(char *fmt, ...) {
  // Prints option error message formatted by fmt and exits
  char msg_opt[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(msg_opt, sizeof(msg_opt), fmt, args);
  va_end(args);
  fprintf(stderr, "%s\n", msg_opt);
  exit(1);
}

static void run_test(char *name, void (*fn)(void)) {
  int ntotal, nfail;
  xCheckInit();
  fn();
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
  run_test("Create",   TestCreate);
  run_test("Uint32",   TestUint32);
  run_test("Uint64",   TestUint64);
  run_test("Normal",   TestNormal);
  run_test("Int",      TestInt);
  //run_test("Norm",     TestNorm);
  //run_test("Perm",     TestPerm);
  //run_test("Sample",   TestSample);
  run_test("U01",      TestU01);
  //run_test("Numbers_mvn", TestMvn);
  vprint(table_fmt, "TOTAL", NTOTAL - NFAIL, NFAIL);
  return (NFAIL > 0);
}
