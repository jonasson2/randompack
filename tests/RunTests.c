#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

#include "printX.h"
#include "Tests.h"
#include "xCheck.h"
#include "test_util.h"

static int NTOTAL = 0, NFAIL = 0;
static char *headr_fmt = "%-20s %8s %8s\n";
static char *table_fmt = "%-20s %8d %8d\n";
int TESTVERBOSITY = 0; // External
int TESTFLOAT = 1; // External

static void print_help(void) {
  puts("Usage: RunTests [options]\n"
       "Options:\n"
       "  -h       Show this help message\n"
       "  -Q       Quick run (all sample counts set to default/20)\n"
       "  -C       Comprehensive run (all sample counts set to default*20)\n"
       "  -v       Verbose tests\n"
       "  -vv      More verbosity\n"
       "  -vvv     Even more verbosity\n"
       "  -d       Double-only tests in TestContinuous\n"
       "  -f N     Set N_STAT_FAST (u01, unif, norm, exp; default 100000)\n"
       "  -s N     Set N_STAT_SLOW (other distributions; default 20000)\n"
       "  -c N     Set N_BAL_CNTS (uintXX bucket counts; default 5000000)\n"
       "  -b N     Set N_BAL_BITS (uintXX & int bit counts; default 40000)\n");
}

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
  printf("%s\n", msg_opt);
  exit(1);
}

static int parse_posint(const char *s) {
  char *end = 0;
  long v = strtol(s, &end, 10);
  if (!s[0] || (end && end[0]) || v <= 0 || v > INT_MAX)
    option_error("Bad integer '%s'", s);
  return (int)v;
}

static void set_quick_sizes(void) {
  N_STAT_FAST = N_STAT_FAST_DEFAULT/20;
  N_STAT_SLOW = N_STAT_SLOW_DEFAULT/20;
  N_BAL_CNTS = N_BAL_CNTS_DEFAULT/20;
  N_BAL_BITS = N_BAL_BITS_DEFAULT/20;
}

static void set_comprehensive_sizes(void) {
  N_STAT_FAST = 20*N_STAT_FAST_DEFAULT;
  N_STAT_SLOW = 20*N_STAT_SLOW_DEFAULT;
  N_BAL_CNTS = 20*N_BAL_CNTS_DEFAULT;
  N_BAL_BITS = 20*N_BAL_BITS_DEFAULT;
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
  char *optstring = ":vQChf:s:c:b:d";
  int c;
  while ((c = getopt(argc, argv, optstring)) != -1) {
    switch (c) {
      case 'h': print_help(); return 0;
      case 'Q': set_quick_sizes(); break;
      case 'C': set_comprehensive_sizes(); break;
      case 'v': TESTVERBOSITY++; break;
      case 'd': TESTFLOAT = 0; break;
      case 'f': N_STAT_FAST = parse_posint(optarg); break;
      case 's': N_STAT_SLOW = parse_posint(optarg); break;
      case 'c': N_BAL_CNTS = parse_posint(optarg); break;
      case 'b': N_BAL_BITS = parse_posint(optarg); break;
      case '?': option_error("Unknown option -%c", optopt);
                // fallthrough
      case ':': option_error("Missing argument for -%c", optopt);
    }
  }
  if (optind < argc) option_error("Unexpected argument %s", argv[optind]);
  if (TESTVERBOSITY <= 1) printOff();
  vprint("\n");
  vprint(headr_fmt, "TEST OF", "PASSED", "FAILED");
  run_test("Helpers",     TestHelpers);
  run_test("Openlibm",    TestOpenlibm);
  run_test("FullMantissa",TestFullMantissa);
  run_test("Bitexact",    TestBitexact);
  run_test("Avx2",        TestAvx2);
  run_test("Create",      TestCreate);
  run_test("Seed",        TestSeed);
  run_test("Buffer",      TestBuffer);
  run_test("SetState",    TestSetState);
  run_test("Serialize",   TestSerialize);
  run_test("Uint8",       TestUint8);
  run_test("Uint16",      TestUint16);
  run_test("Uint32",      TestUint32);
  run_test("Uint64",      TestUint64);
  run_test("Reference",   TestReference);
  run_test("X256ppsimd",  TestX256ppsimd);
  run_test("Sfc64simd",   TestSfc64simd);
  run_test("Ranluxpp",    TestRanluxpp);
  run_test("Int",         TestInt);
  run_test("LongLong",    TestLongLong);
  run_test("Perm",        TestPerm);
  run_test("Sample",      TestSample);
  run_test("Unif",        TestUnif);
  run_test("Normal",      TestNorm);
  run_test("Exp",         TestExp);
  run_test("Continuous",  TestContinuous);
  run_test("Dpstrf",      TestDpstrf);
  run_test("Mvn",         TestMvn);
  vprint(table_fmt, "TOTAL", NTOTAL - NFAIL, NFAIL);
  return (NFAIL > 0);
}
