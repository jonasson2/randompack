// RunTestNo128.c
#include <stdio.h>
#include <string.h>
#include "test_util.h"

void TestCreate(void);

int main(int argc, char **argv) {
  bool verbose = false;
  for (int i=1; i<argc; i++) {
    if (strcmp(argv[i], "-v") == 0) verbose = true;
  }
  if (verbose) printf("Running TestCreate under simulated no-128 build.\n");
  TestCreate();
  return 0;
}
