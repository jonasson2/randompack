# Randompack.jl

Randompack.jl provides Julia bindings to the C library Randompack, a random
number generation toolkit which also includes interfaces to R, Fortran, and
Python. Randompack exposes a collection of modern RNG engines, including
xoshiro256++/**, PCG64 DXSM, sfc64, Philox, and ChaCha20, together with a range
of probability distributions, both integer and continuous. The library allows
matching random draws across platforms and architectures, a feature not
available in other packages. It provides unbounded and bounded integer draws,
permutations, samples, and 14 continuous distributions, ranging from basic
ones (uniform, normal, exponential), through important more advanced (beta,
gamma), to some not availaible in Julia's standard distribution library
(skew-normal). Multivariate normal sampling is also supported. Through SIMD
instructions on modern CPUs, the inherently fast default engine, xoshiro256++,
provides random number generation that closely matches or exceeds native Julia's
speed.

## Requirements

- 64-bit Julia (`Sys.WORD_SIZE == 64`)
- `librandompack` available on the system (see *Library discovery* below)

## Loading

```julia
using Randompack
```

## Available RNG engines

| Engine      | Description                                                |
|-------------|------------------------------------------------------------|
| `x256++simd`| xoshiro256++ with SIMD acceleration (default)              |
| `x256++`    | xoshiro256++ (Vigna and Blackman, 2018)                    |
| `x256**`    | xoshiro256** (Vigna and Blackman, 2018)                    |
| `xoro++`    | xoroshiro128++ (Vigna and Blackman, 2016)                  |
| `x128+`     | xorshift128+ (Vigna, 2014)                                 |
| `pcg64`     | Permuted Congruential Generator PCG64 DXSM (O’Neill, 2014) |
| `sfc64`     | Small Fast Chaotic PRNG (Chris Doty-Humphrey, 2013)        |
| `cwg128`    | Collatz-Weyl Generator cwg128-64 (Działa, 2022)            |
| `philox`    | Philox-4×64 (Salmon and Moraes, 2011)                      |
| `squares`   | squares64 (Widynski, 2021)                                 |
| `chacha20`  | ChaCha20 (Bernstein, 2008)                                 |
| `system`    | Operating-system entropy source                            |

The available engines along with descriptions can be queried programmatically with:

```julia
Randompack.engines()
```

## Basic setup

```julia
rng = rng_create()                # default engine (x256++simd)
rng = rng_create("pcg64")         # select engine explicitly
rng_seed!(rng, 123)               # deterministic seed
Randompack.randomize!(rng)        # seed from system entropy
rng2 = Randompack.duplicate(rng)  # independent copy with identical state
```

## Continuous distributions

All continuous distributions are available for `Float64` and `Float32`.

### Allocating forms

```julia
x = random_unif(rng, 100)
y = random_normal(rng, Float32, 100; mu = 2.0f0, sigma = 3.0f0)
z = random_beta(rng, 50; a = 2.0, b = 5.0)
```

### In-place forms

```julia
A = zeros(10, 10)
B = similar(A)
random_unif!(rng, A)
random_exp!(rng, B; scale = 2.0)
```

### Available distributions

| Distribution        | Allocating           | In-place              |
|---------------------|----------------------|-----------------------|
| uniform             | `random_unif`        | `random_unif!`        |
| normal              | `random_normal`      | `random_normal!`      |
| skew-normal         | `random_skew_normal` | `random_skew_normal!` |
| lognormal           | `random_lognormal`   | `random_lognormal!`   |
| exponential         | `random_exp`         | `random_exp!`         |
| gamma               | `random_gamma`       | `random_gamma!`       |
| chi-square          | `random_chi2`        | `random_chi2!`        |
| beta                | `random_beta`        | `random_beta!`        |
| t                   | `random_t`           | `random_t!`           |
| F                   | `random_f`           | `random_f!`           |
| gumbel              | `random_gumbel`      | `random_gumbel!`      |
| pareto              | `random_pareto`      | `random_pareto!`      |
| weibull             | `random_weibull`     | `random_weibull!`     |
| multivariate normal | `random_mvn`         | `random_mvn!`         |

## Discrete distributions

### Allocating forms

```julia
x = random_int(rng, 3:10, 100)
y = random_int(rng, Int8(0):Int8(10))
z = random_int(rng, (10,10))
p = random_perm(rng, 10, Int32)
s = random_sample(rng, 20, 5, UInt16)
```

### In-place forms

```julia
A = Matrix{UInt16}(undef, 5, 5)
q = Vector{UInt8}(undef, 8)
t = Vector{UInt16}(undef, 5)
random_int!(rng, A, 0:200)
random_perm!(rng, q)
random_sample!(rng, t, 20)
```

## Multivariate normal

```julia
Sigma = [1.0 0.2; 0.2 2.0]
X = zeros(100, 2);
random_mvn!(rng, X, Sigma; mu=[1.0, 2.0])  # specified mean
Y = random_mvn(rng, 5, Sigma)              # zero mean
```

## RNG management (advanced)

These functions expose lower-level control over RNG state and configuration.

```julia
rng_seed!(rng, 123; spawn_key = [1, 2])
bytes = Randompack.serialize(rng)
Randompack.deserialize!(rng, bytes)
Randompack.full_mantissa!(rng, true)
engines = Randompack.engines()
```

Raw engine state setters (engine-specific, integer arguments):

```julia
rng = rng_create()
Randompack.set_state!(rng; state=[1, 2])
Randompack.philox_set_state!(rng; ctr = [1, 2, 3, 4], key = [5, 6])
Randompack.squares_set_state!(rng; ctr = 1, key = 2)
Randompack.pcg64_set_state!(rng; state = 1, inc = 2)
```

## Library discovery

By default, Randompack.jl loads `librandompack` using the system dynamic loader. You can override the library location by setting:

```julia
ENV["RANDOMPACK_LIB"] = "/path/to/librandompack"
```

## Notes

- RNG objects are finalized automatically when no longer referenced.
- For parallel workloads, use one RNG instance per thread or task.
