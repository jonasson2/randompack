# Randompack

## Introduction

Randompack is a library for random number generation written in C. It is
intended to be robust, portable, and straightforward to use, while placing
strong emphasis on correctness and reproducibility. Interfaces are provided for
C, Fortran, R, and Python, with compatible streams across languages under the
same generator choice and seed. The library provides a range of distributions,
including raw bit-streams, bounded and interval-based integers, permutations and
sampling without replacement, uniform and normal variates, and several other
commonly used continuous distributions. All distribution algorithms are
implemented directly from their original or standard published descriptions,
with care taken to avoid undocumented shortcuts or ad-hoc modifications.

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
when drawing large numbers of variates (using a fixed 1 KB buffer chosen for
performance). The library is thread-safe, released under the MIT licence, and
built using the Meson build system with Ninja as the default backend. The C11
core is intended to build cleanly on all major platforms and to serve both as a
reliable standalone library and as a common foundation for language bindings.

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

## Support functions

A small set of support functions is provided to

- create and free generator instances
- report the last error condition
- initialize generators from a seed
- set generator state explicitly
- serialize and deserialize generator state

State serialization supports checkpointing and allows simulations to be stopped
and restarted exactly.

## Random number generators

Randompack offers several underlying random number generators, referred to as
*engines*. ChaCha20 is provided as a cryptographically secure generator. Philox
and squares64 are counter-based generators, while the remaining engines are
state-based. A system-provided generator is also available.

- xoshiro256++ (default; Vigna and Blackman, 2018–2019)
- xoshiro256** (Vigna and Blackman, 2018–2019)
- xoroshiro128++ (Vigna and Blackman, 2016)
- xorshift128+ (Vigna, 2014)
- PCG64-DXSM (O’Neill, 2014)
- cwg128 (Działa, 2022)
- Philox 4×64 (Salmon and Moraes, 2011)
- squares64 (Widynski, 2021)
- ChaCha20 (Bernstein, 2008)
- system generator (operating-system–provided entropy source)

## Verification

Correctness of the underlying random number generators is a central design goal
of Randompack. For most engines, the raw bit-stream output is verified directly
against independent reference implementations from authoritative upstream
sources or widely used mainstream libraries.

Philox is verified against the Random123 reference implementation, ChaCha20
against the official RFC 8439 test vectors, PCG64 against NumPy, and xoshiro256++
and xoshiro256** against Rust’s standard implementation. For engines where no
widely accepted reference streams are readily available (xorshift128+,
xoroshiro128++, cwg128, and squares64), verification relies on careful
implementation of the published algorithms together with the distributional
tests described in the next section.

## Testing

In addition to verification handled by the test suite, both API behavior
and distributional correctness are checked. The tests include checking that

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
