#ifndef XCHECK_H
#define XCHECK_H

#include <stdio.h>
#include <string.h>
#include "xCheck.h"

static int NTOTAL = 0;
static int NFAIL = 0;
static char commonmsg[60] = "";
static char addmsg[60] = "";

void xCheckFunc(const char *message, char* file, int line) {
  // Print filename, line number, and upto two messages.
  char fmt[] = "%s:%d: %s test failed: %s is false";
  fprintf(stderr, fmt, file, line, commonmsg, message);
  if (strlen(addmsg) > 0) fprintf(stderr, " (%s)", addmsg);
  fprintf(stderr, "\n");
  fflush(stderr);
  NTOTAL += 1;
  NFAIL += 1;
}

void xCheckOK(void) {
  NTOTAL += 1;
}

void xCheckInit(const char *msg) { // set common message to msg and NFAIL to 0
  strcpy(commonmsg, msg);
  addmsg[0] = 0;
  NTOTAL = 0;
  NFAIL = 0;
}

void xCheckAddMsg(const char *amsg) { // set additional message to amsg
  strcpy(addmsg, amsg);
}

int xCheckNFailures(void) {
  return NFAIL;
}

int xCheckNTotal(void) {
  return NTOTAL;
}

#endif
