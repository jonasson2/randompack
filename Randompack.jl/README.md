# Randompack.jl

This package provides Julia bindings to the C library Randompack, a random number
generation toolkit that also includes interfaces for R, Fortran, and Python. Randompack
exposes a collection of modern RNG engines, including xoshiro256++/**, PCG64 DXSM, sfc64,
ranlux++, Philox, and ChaCha20, together with a range of probability distributions, both
integer and continuous. The library allows matching random draws across platforms and
supported language interfaces. It provides unbounded and bounded integer draws,
permutations, sampling without replacement, and 14 continuous distributions, ranging from
basic ones (uniform, normal, exponential), through commonly used distributions (beta,
gamma), to some not available in Julia’s standard distribution library (skew-normal).
Multivariate normal sampling is also supported.

Through SIMD instructions on modern CPUs, the inherently fast default engine,
xoshiro256++, delivers performance that matches or exceeds native Julia’s random number
generation.

For more information, including implementation details, benchmarking results, and
documentation of engines and distributions, see the main project readme file at
https://github.com/jonasson2/randompack. The same page also links to DEVELOPMENT.md, which
contains setup and development instructions, including details specific to the Julia
interface.

## Cross-platform consistency

Given the same engine and seed, samples obtained on different platforms (programming
language/computer/compiler/OS/architecture) agree. For uniform, normal, exponential, and
integer distributions the agreement is bit-exact (x == y holds). For the remaining
distributions, samples agree to within ca. 2 ulp. If the `bitexact` parameter is set to
`true` the agreement is bit-exact for all distributions.

## Usage

### Installation, setup, and seeding
```julia
] add Randompack   # add to active environment (run once per environment)
using Randompack   # load package into session

rng = rng_create()                    # default engine (x256++simd)
rng = rng_create("pcg64")             # specified engine; rng is randomized by default
Randompack.engines()                  # list available engines
rng_seed!(rng, 123)                   # deterministic seed
rng_seed!(rng, 123; spawn_key=[1, 2]) # independent substreams
Randompack.randomize!(rng)            # seed from system entropy
rng2 = Randompack.duplicate(rng)      # identical independent copy
```

### Continuous distributions
```julia
x = random_unif(rng, 100)                 # 100 draws from U(0,1)
y = random_unif(rng, 100; a=2, b=5)       # 100 draws from U(2,5)
s = random_unif(rng, 1)                   # scalar draw (length-1 array)
z = random_normal(rng, 5)                 # 5 standard normal draws
t = random_normal(rng, 5; mu=2, sigma=3)  # 5 draws from N(2,3)
u = random_beta(rng, 50; a=2, b=5)        # 50 draws from the Beta(2,5) distribution
v = random_normal(rng, Float32, 5)        # single precision
random_unif!(rng, x)                      # use shape and data type of x
```

### Discrete distributions
```julia
x = random_int(rng, 1:6, 100)      # integers in [1,6] (inclusive)
p = random_perm(rng, 10)           # permutation of 1:10
s = random_sample(rng, 20, 5)      # 5-element sample from 1:20 (without replacement)
```

### Multivariate normal
```julia
Sigma = [1.0 0.2; 0.2 2.0]
X = random_mvn(rng, 100, Sigma)                  # zero mean
Y = random_mvn(rng, 50, Sigma; mu=[1.0, 2.0])    # specified mean
Z = zeros(100, 2)                                # 2 columns
random_mvn!(rng, Z, Sigma)                       # Sigma must be 2×2
```

### State control and serialization
```julia
rngx = rng_create("x256**")
rngp = rng_create("philox")
rngq = rng_create("pcg64")
rngs = rng_create("sfc64")
rngc = rng_create("chacha20")
rngz = rng_create("squares")
rngr = rng_create("ranlux++")
Randompack.set_state!(rngx; state=[1,2,3,4])                  # general state setter
Randompack.set_state!(rngp; state=[1,2,3,4,0,0])              # set full Philox state
Randompack.philox_set_key!(rngp; key=[4,6])                   # set Philox key
Randompack.jump!(rngx, 128)                                   # jump state by 2^128 steps
Randompack.pcg64_set_inc!(rngq; inc=[3, 5])                   # change PCG increment
Randompack.set_state!(rngs; state=[1,2,3,17])                # set full sfc64 state
Randompack.sfc64_set_abc!(rngs; abc=[7,11,13])               # update only a, b, c
Randompack.chacha_set_nonce!(rngc; nonce=[7,11,13])          # change ChaCha20 nonce
Randompack.set_state!(rngz; state=[3,0])                     # set full Squares state
Randompack.squares_set_key!(rngz; key=4)                     # set Squares key
Randompack.jump!(rngr, 32)                                    # jump state by 2^32 steps

rngy = rng_create("x256**")                      # engines must match
state = Randompack.serialize(rngx)               # copy engine state of rngx
Randompack.deserialize!(rngy, state)             # and put in rngy

Randompack.full_mantissa!(rng, true)      # enable 53-bit mantissa (52-bit is default)
rng = rng_create(bitexact=true)           # make agreement across platforms exact
rng = rng_create("philox"; bitexact=true) # exact agreement with specified engine
``` 
