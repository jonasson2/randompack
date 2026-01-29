# Randompack.jl

Randompack.jl provides access to the `randompack` C library from Julia. Use
`rng_create()` to construct an RNG instance, and specify the underlying engine
with `rng_create(engine)` where `engine` is a string naming the RNG (see the
list of engines below). The returned RNG object provides methods for drawing
random variates from a variety of distributions. The RNG can be configured with
setup and state-control methods (e.g., `rng_seed!`, `Randompack.randomize!`).
Multiple independent RNG objects can be used in parallel across processes or
threads.

## Requirements

- 64-bit Julia (`Sys.WORD_SIZE == 64`)
- `librandompack` available on the system (see `RANDOMPACK_LIB` below)

## Loading

```julia
using Randompack
```

## List of engines

x256++    xoshiro256++ (default; Vigna and Blackman, 2018)
x256**    xoshiro256** (Vigna and Blackman, 2018)
xoro++    xoroshiro128++ (Vigna and Blackman, 2016)
x128+     xorshift128+ (Vigna, 2014)
pcg64     PCG64 DXSM (O'Neill, 2014)
cwg128    cwg128-64 (Działa, 2022)
philox    Philox-4×64 (Salmon and Moraes, 2011)
squares   squares64 (Widynski, 2021)
chacha20  ChaCha20 (Bernstein, 2008)
system    Operating-system–provided entropy source

## Setup

```julia
rng = rng_create()                   # default engine (x256++simd)
rng = rng_create("pcg64")            # specific engine

rng_seed!(rng, 123)                  # deterministic seed
rng_seed!(rng, 123; spawn_key=UInt32[1,2])
Randompack.randomize!(rng)           # randomize from system entropy

rng2 = Randompack.duplicate(rng)     # duplicate (same state)

bytes = Randompack.serialize(rng)    # serialize
Randompack.deserialize!(rng, bytes)

Randompack.full_mantissa!(rng, true) # enable 53-bit mantissas for doubles
```

## Continuous distributions

All continuous distributions are available for `Float64` and `Float32`.
Allocating forms:

```julia
x = random_unif(rng, Float64, 100; a=0, b=1)
x = random_normal(rng, Float32, 100; mu=0, sigma=1)
```

In-place forms:

```julia
A = Matrix{Float64}(undef, 10, 10)
random_unif!(rng, A; a=0, b=1)
```

Available distributions (allocating and in-place):

- `random_unif`       `random_unif!`
- `random_normal`     `random_normal!`
- `random_lognormal`  `random_lognormal!`
- `random_exp`,       `random_exp!`
- `random_gamma`,     `random_gamma!`
- `random_chi2`,      `random_chi2!`
- `random_beta`,      `random_beta!`
- `random_t`,         `random_t!`
- `random_f`,         `random_f!`
- `random_gumbel`,    `random_gumbel!`
- `random_pareto`,    `random_pareto!`
- `random_weibull`,   `random_weibull!`

## Discrete distributions

### Uniform integers over a type

```julia
x = random_int(rng, Int32, 100)
A = Matrix{UInt16}(undef, 5, 5)
random_int!(rng, A)
```

### Uniform integers over a range

```julia
x = random_int(rng, Int8(0):Int8(10), 100)
y = random_int(rng, Int8(0):Int8(10))
```

### Permutations and samples (1-based)

```julia
p = random_perm(rng, 10, Int32)
q = Vector{UInt8}(undef, 8)
random_perm!(rng, q)

s = random_sample(rng, 20, 5, UInt16)
random_sample!(rng, s, 20)
```

## State control (advanced)

These functions set raw engine state. All arguments use unsigned integer types.

```julia
Randompack.set_state!(rng, UInt64[1,2,3,4])

Randompack.philox_set_state!(rng; ctr=(0x1,0x2,0x3,0x4), key=(0x5,0x6))
Randompack.squares_set_state!(rng; ctr=0x1, key=0x2)
Randompack.pcg64_set_state!(rng; state=UInt128(1), inc=UInt128(2))
```

## Multivariate normal

```julia
Sigma = [1.0 0.2; 0.2 2.0]
X = random_mvn(rng, 5, Sigma)
random_mvn!(rng, X, Sigma; mu=[1.0, 2.0])
```

## Library discovery

By default the wrapper uses `librandompack` from the system loader. You can
override this by setting:

```
ENV["RANDOMPACK_LIB"] = "/path/to/librandompack"
```

## Notes

- RNG objects are finalized automatically when no longer referenced.
- Use one RNG per thread if you need parallel streams.
