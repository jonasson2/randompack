# Randompack

## Introduction

Randompack is a library for random number generation written in C. It is
intended to be robust, portable, fast, and easy to use, while placing emphasis
on correctness and reproducibility. Interfaces are provided for C, Fortran, R,
and Python, with compatible streams across languages under the same generator
choice and seed. The library provides a range of distributions, including raw
bit-streams, bounded and interval-based integers, permutations and sampling
without replacement, uniform and normal variates, and several other commonly
used continuous distributions. All distribution algorithms are implemented
directly from their original or standard published descriptions, with care taken
to avoid undocumented shortcuts or ad-hoc modifications.

The library includes several modern random number generators, including multiple
members of Vigna and Blackman’s xor family, O’Neill’s PCG64, the Philox generator
by Salmon and Moraes, and Bernstein’s ChaCha20. These generators are implemented
to match their published reference definitions. Where possible, outputs are
compared directly against upstream implementations and known reference streams
to verify correctness. A central design goal of Randompack is confidence in
correctness, supported by extensive testing at multiple levels, including unit
tests, distributional checks, and direct comparisons with authoritative upstream
engines and reference implementations.

Randompack is designed for high-performance bulk generation. It uses buffering
and inlining in tight, low-overhead inner loops, minimizing per-sample overhead
when drawing large numbers of variates (using a fixed 2 KB buffer chosen for
performance). The library is thread-safe, released under the MIT licence, and
built using the Meson build system with Ninja as the default backend. The C11
core is intended to build cleanly on all major platforms and to serve both as a
reliable standalone library and as a common foundation for language bindings.

The public header file `randompack.h` serves as a compact reference for the C
API: all user-facing functions are declared there, with comments describing the
role of each parameter.

## Distributions

All distributions are implemented independently of the underlying random number
generator, allowing any supported distribution to be used with any supported
generator. The continuous distributions are provided in both double- and
single-precision (float) variants.

- raw random bit streams (uint8, uint16, uint32, and uint64)
- bounded unsigned integers
- signed integers uniform on an interval
- random permutations
- sampling without replacement
- continuous uniform
- normal (standard and general)
- lognormal
- exponential
- gamma
- beta
- chi-square
- Student’s t
- F
- Weibull
- Gumbel
- Pareto
- multivariate normal (double precision only)

## Random number generators

Randompack offers several underlying random number generators, referred to as
*engines*. ChaCha20 is provided as a cryptographically secure generator. Philox
and squares64 are counter-based generators, while the remaining engines are
state-based. A system-provided generator is also available. When selecting an
engine the names are case-insensitive.

- x256++    xoshiro256++ (default; Vigna and Blackman, 2018)
- x256**    xoshiro256** (Vigna and Blackman, 2018)
- xoro++    xoroshiro128++ (Vigna and Blackman, 2016)
- x128+     xorshift128+ (Vigna, 2014)
- pcg64     PCG64 DXSM (O’Neill, 2014)
- cwg128    cwg128-64 (Działa, 2022)
- sfc64     sfc64 (Chris Doty-Humphrey, 2013)
- philox    Philox-4×64 (Salmon and Moraes, 2011)
- squares   squares64 (Widynski, 2021)
- chacha20  ChaCha20 (Bernstein, 2008)
- system    Operating-system–provided entropy source

## Support functions

A small set of support functions is provided to

- create and free generator instances
- report the last error condition
- initialize generators from a seed
- enable or disable full mantissas for double-precision draws
- set generator state explicitly
- serialize and deserialize generator state

Seeding uses a high-quality seed expansion mechanism based on Melissa O'Neill's
seed_seq_fe128, also adopted by NumPy to initialize its random number
generators. The mechanism supports reproducible construction of independent
substreams via optional spawn keys. For the counter based generators, the
explicit state setting allows explicit setting of both counter and key. State
serialization supports checkpointing and allows simulations to be stopped and
restarted exactly.

## Verification

Correctness of the underlying random number generators is a central design goal
of Randompack. For most engines, the raw bit-stream output is verified directly
against independent reference implementations from authoritative upstream
sources or widely used mainstream libraries.

Philox is verified against the Random123 reference implementation, ChaCha20
against the official RFC 8439 test vectors, PCG64 against NumPy, and xoshiro256++
and xoshiro256** against Rust’s standard implementation. For engines where no
widely accepted reference streams are readily available (xorshift128+,
xoroshiro128++, cwg128, sfc64, and squares64), verification relies on careful
implementation of the published algorithms together with the distributional
tests described in the next section.

## Testing

In addition to the verification described above, the test suite checks both API
behavior and distributional correctness. The tests include checking that

- zero-length buffers are handled correctly
- invalid inputs return an error
- same seed produces identical output
- different seeds produce different outputs
- drawn variates are contained in each distribution's support
- the frequency of each bit in bit streams is statistically balanced
- counts across all values of bounded and interval integer draws are balanced
- for continuous distributions, the corresponding PIT-transformed U01 distribution
  exhibits balanced bins, and its mean, variance, skewness, and kurtosis pass
  statistical tests
  
The test framework supports increasing the size of generated test vectors
(currently ranging from `2e4` to `5e5`) for more comprehensive, longer-running
test suites.

## External statistical validation

Two external statistical validation suites are supported. These may be used to
check the raw bit streams generated by randompack's engines.

- TestU01 (L'Ecuyer and Simard 2007) via `TestU01Driver`
- PractRand (Doty-Humphrey 2013) via `RawStream`

## SIMD support [this requires editing]
The program advances four independent RNG streams. On Arm (Apple M-series)
processors, SIMD vector instructions provide two lanes, and each lane uses
instruction-level parallelism (ILP) via loop unrolling to give two independent
execution chains. On modern x86-64 processors with AVX2, four SIMD lanes are
used instead. On other processors, four-way loop unrolling is used to advance
the four streams.

## Timing

Randompack includes a small set of benchmarking programs intended to measure
raw generator throughput and per-value distribution costs. All benchmarks are
implemented as standalone C programs with a small number of command-line
options; details are available via the `-h` option.

- `TimeEngines` measures the throughput of each random number generator in
  MB/s by repeatedly drawing blocks of 1024 `uint64` values.
- `TimeIntegers` reports time per generated value (in ns) for selected integer
  distributions.
- `TimeDistributions` reports time per generated value (in ns) for each
  continuous distribution in double and float, using the default engine
  (`xoshiro256++`).

## Building and installation
Randompack relies on the Meson/Ninja build system. Install these programs if
they are not already available.

To build an optimized release version of Randompack:

        meson setup build --buildtype=release
        meson compile -C build

To run the test suite:

        meson test -C build

To install the library and headers into the default installation directory
(usually `/usr/local`):

        sudo meson install -C build

To prescribe a local installation directory, include a --prefix option when
configuring the build, for example:

        meson setup build --buildtype=release --prefix=$HOME/opt
        meson install -C build

After installation, Randompack can be used by other projects via pkg-config:

        pkg-config --cflags --libs randompack

## C API overview (by example)

The examples subfolodr contains additional example programs.

### Minimal example: N(0,1), automatically randomized rng, default engine

        #include <stdio.h>
        #include "randompack.h"

        int main(void) {
          randompack_rng *rng = randompack_create(0);
          double x[5];
          randompack_norm(x, 5, rng);
          for (int i = 0; i < 5; i++) printf("%g\n", x[i]);
          randompack_free(rng);
          return 0;
        }

### N(3,2), PCG64 engine, seeded with 42

        #include <stdio.h>
        #include "randompack.h"

        int main(void) {
          randompack_rng *rng = randompack_create("pcg64");
          randompack_seed(42, 0, 0, rng);
          double x[5];
          randompack_normal(x, 5, 3.0, 2.0, rng);
          for (int i = 0; i < 5; i++) printf("%g\n", x[i]);
          randompack_free(rng);
          return 0;
        }

### Secondary distributions

        #include <stdio.h>
        #include "randompack.h"

        int main(void) {
          randompack_rng *rng = randompack_create(0);
          randompack_seed(7, 0, 0, rng);
          double x[5];
          randompack_chi2(x, 5, 5, rng);
          randompack_weibull(x, 5, 2, 3, rng);
          randompack_skew_normal(x, 5, 0, 1, 1, rng);
          randompack_free(rng);
          return 0;
        }

### Integer range example

        #include <stdio.h>
        #include "randompack.h"

        int main(void) {
          randompack_rng *rng = randompack_create(0);
          randompack_seed(7, 0, 0, rng);
          int xi[5];
          long long xl[5];
          randompack_int(xi, 5, -3, 8, rng);
          randompack_long_long(xl, 5, -10, 10, rng);
          for (int i = 0; i < 5; i++)
            printf("%d %lld\n", xi[i], xl[i]);
          randompack_free(rng);
          return 0;
        }

### Raw bitstream example

        #include <stdio.h>
        #include "randompack.h"

        int main(void) {
          randompack_rng *rng = randompack_create(0);
          long long xi[2];
          unsigned char xb[1];
          randompack_raw(xi, sizeof(xi), rng);
          randompack_raw(xb, sizeof(xb), rng);
          printf("%lld %lld %u\n", xi[0], xi[1], (unsigned)xb[0]);
          randompack_free(rng);
          return 0;
        }

### Example with full error checking
        #include <stdio.h>
        #include "randompack.h"

        int main(void) {
          bool ok;
          double x[10];
          randompack_rng *rng = 0;
          rng = randompack_create("x256++");
          if (!rng) {
            fprintf(stderr, "rng creation failed\n");
            return 1;
          }
          ok = randompack_seed(12345, 0, 0, rng);
          if (!ok) goto end;
          ok = randompack_norm(x, 10, rng);
          if (!ok) goto end;
          for (int i = 0; i < 10; i++) printf("% .17g\n", x[i]);

        end:
          char *msg = randompack_last_error(rng);
          if (msg)
            fprintf(stderr, "randompack error: %s\n", msg);
          randompack_free(rng);
          return msg ? 1 : 0;
        }

### Threads example demonstrating spawn key seeding

In this example each thread uses the same base seed but a different spawn key
(the thread id), producing reproducible independent substreams. Note that the
example relies on assert() for simplicity – if compiled with -DNDEBUG these
checks become inactive. The example relies on pthreads, usually not available on
Windows.

        // Simple pthread example.
        // Launch M threads. Each thread derives an independent RNG stream from a
        // common seed via a spawn key and computes max(U(0,1)) over N draws.

        #include <stdint.h>
        #include <stdio.h>
        #include <pthread.h>

        #include "randompack.h"

        enum { M = 10, N = 100000 };
        static int seed = 42;

        typedef struct {
          int stream_id;
          double result;
        } job;

        static void *worker(void *arg) {
          job *j = arg;
          randompack_rng *rng = randompack_create(0);
          if (!rng) return 0;
          uint32_t spawn_key[1];
          spawn_key[0] = j->stream_id;
          if (!randompack_seed(seed, spawn_key, 1, rng)) {
            randompack_free(rng);
            return 0;
          }
          double m = 0;
          for (int i = 0; i < N; i++) {
            double u;
            randompack_u01(&u, 1, rng);
            if (u > m) m = u;
          }
          j->result = m;
          randompack_free(rng);
          return 0;
        }

        int main(void) {
          pthread_t th[M];
          job jobs[M];
          for (int i = 0; i < M; i++) {
            jobs[i].stream_id = i;
            jobs[i].result = 0;
            pthread_create(&th[i], 0, worker, &jobs[i]);
          }
          for (int i = 0; i < M; i++)
            pthread_join(th[i], 0);
          for (int i = 0; i < M; i++)
            printf("stream %d: max = %.6f\n", i, jobs[i].result);
          return 0;
        }
