# Randompack.jl

This package provides Julia bindings to the C library Randompack, a random number
generation toolkit that also includes interfaces for R, Fortran, and Python. Randompack
exposes a collection of modern RNG engines, including xoshiro256++/**, PCG64 DXSM, sfc64,
Philox, and ChaCha20, together with a range of probability distributions, both integer and
continuous. The library allows matching random draws across platforms and supported
language interfaces. It provides unbounded and bounded integer draws, permutations,
sampling without replacement, and 14 continuous distributions, ranging from basic ones
(uniform, normal, exponential), through commonly used distributions (beta, gamma), to some
not available in Julia’s standard distribution library (skew-normal). Multivariate normal
sampling is also supported.

Through SIMD instructions on modern CPUs, the inherently fast default engine,
xoshiro256++, delivers performance that closely matches or exceeds native Julia’s random
number generation.

For more information, including implementation details, benchmarking results, and
documentation of engines and distributions, see the main project readme file at
https://github.com/jonasson2/randompack.

## Usage

### Installation, setup, and seeding
```julia
] add Randompack   # run once
using Randompack

rng = rng_create()                    # default engine (x256++simd)
rng = rng_create("pcg64")             # specified engine; rng is randomized by default
Randompack.engines()                  # list available engines
rng_seed!(rng, 123)                   # deterministic seed
rng_seed!(rng, 123; spawn_key=[1,2])  # independent substreams
Randompack.randomize!(rng)            # seed from system entropy
rng2 = Randompack.duplicate(rng)      # identical independent copy
```

### Continuous distributions
```julia
x = random_unif(rng, 100)                     # 100 draws from U(0,1)
y = random_unif(rng, 100; a=2.0, b=5.0)       # 100 draws from U(2,5)
s = random_unif(rng)                          # scalar draw
z = random_normal(rng, 5)                     # 5 standard normal draws
t = random_normal(rng, 5; mu=2.0, sigma=3.0)  # 5 draws from N(2,3)
u = random_beta(rng, 50; a=2.0, b=5.0)        # 50 draws from the Beta(2,5) distribution
y32 = random_normal(rng, Float32, 100)        # Float32

A = zeros(10); E = zeros(10)
random_unif!(rng, A)                          # in-place form
random_exp!(rng, E; scale=2.0)                # 
```

### Discrete distributions
```julia
x = random_int(rng, 1:6, 100)          # integers in [1,6]
p = random_perm(rng, 10)               # permutation of 1:10
s = random_sample(rng, 20, 5)          # 5-element sample from 1:20

v = Vector{Int}(undef, 10)
random_perm!(rng, v)                   # in-place permutation
```

### Multivariate normal
```julia
Sigma = [1.0 0.2; 0.2 2.0]
X = random_mvn(rng, 100, Sigma)                  # zero mean
Y = random_mvn(rng, 50, Sigma; mu = [1.0, 2.0])  # specified mean
Z = zeros(100, 2)
random_mvn!(rng, Z, Sigma; mu = [1.0, 2.0])      # in-place
```

### State control and serialization
```julia
rngx = rng_create("x256**")
rngp = rng_create("philox")
set_state(rngx; state=[1,2,3,4])                 # general state setter
philox_set_state(rng; ctr=[1,2,3,4], key=[4,6])  # philox state setter

rngy = rng_create('x256**')
bytes = Randompack.serialize(rngx)
Randompack.deserialize!(rngy, bytes)

Randompack.full_mantissa!(rng, true)  # enable full 53-bit mantissa (52 bit is default)
```
