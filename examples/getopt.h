// getopt.h — Minimal POSIX getopt() interface
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

#ifndef GETOPT_H_
#define GETOPT_H_

// POSIX-style short-option API (no long options)

extern int optind;   // index of first non-option in argv
extern int optopt;   // option character checked for validity
extern int opterr;   // nonzero enables built-in diagnostics
extern char *optarg; // argument associated with current option

int getopt(int argc, char * const argv[], const char *optstring);

#endif // GETOPT_H_
