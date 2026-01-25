// getopt.c — Minimal POSIX getopt() implementation (short options only)
//
// Derived from the OpenBSD / NetBSD implementation by
//   Todd C. Miller and The NetBSD Foundation, Inc.
// Original code obtained via:
//   https://github.com/alex85k/wingetopt
//
// Copyright (c) 2002 Todd C. Miller <Todd.Miller@courtesan.com>
// Copyright (c) 2000 The NetBSD Foundation, Inc.
// Copyright (c) 2025 Kristján Jónasson  (modifications and trimming)
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHORS DISCLAIM ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "getopt.h"
#include <string.h>
#include <stdio.h>

int opterr = 1;    // if error message should be printed
int optind = 1;    // index into parent argv vector
int optopt = '?';  // character checked for validity
char *optarg;      // argument associated with option

static const char EMSG[] = "";
static char *place = (char *)EMSG;  // option letter processing
static const char recargchar[] = "option requires an argument -- %c\n";
static const char illoptchar[] = "unknown option -- %c\n";

int getopt(int nargc, char * const nargv[], const char *options) {
  const char *oli;
  int optchar;
  if (!options || !nargv) return -1;
  if (!*place) {
    if (optind >= nargc) return -1;
    if (nargv[optind][0] != '-' || nargv[optind][1] == 0) return -1;
    if (nargv[optind][0] == '-' && nargv[optind][1] == '-' && nargv[optind][2] == 0) {
      optind++;
      return -1;
    }
    place = (char *)nargv[optind] + 1;
  }
  optchar = (unsigned char)*place++;
  if (optchar == ':' || !(oli = strchr(options, optchar))) {
    if (!*place) {
      optind++;
      place = (char *)EMSG;
    }
    optopt = optchar;
    if (opterr && *options != ':') fprintf(stderr, illoptchar, optchar);
    return '?';
  }
  if (oli[1] != ':') {
    if (!*place) {
      optind++;
      place = (char *)EMSG;
    }
    optarg = 0;
  } else {
    if (*place) {
      optarg = place;
      optind++;
      place = (char *)EMSG;
    } else {
      optind++;
      if (optind >= nargc) {
        optopt = optchar;
        if (opterr && *options != ':') fprintf(stderr, recargchar, optchar);
        return (*options == ':') ? ':' : '?';
      }
      optarg = nargv[optind];
      optind++;
      place = (char *)EMSG;
    }
  }
  return optchar;
}
