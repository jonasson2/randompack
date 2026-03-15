# randompack

This package provides R bindings to the C library Randompack, a random
number generation toolkit that also includes interfaces for Julia,
Fortran, and Python. Randompack exposes a collection of modern RNG
engines, including xoshiro256++/\*\*, PCG64 DXSM, sfc64, ranlux++, Philox,
and ChaCha20, together with a range of probability distributions, both
integer and continuous. The library allows matching random draws across
platforms and supported language interfaces. It provides unbounded and
bounded integer draws, permutations, sampling without replacement, and
14 continuous distributions, ranging from basic ones (uniform, normal,
exponential), through commonly used distributions (beta, gamma), to more
specialized ones (such as skew-normal). Multivariate normal sampling is
also supported.

Through SIMD instructions on modern CPUs, the inherently fast default
engine xoshiro256++ delivers high throughput for bulk generation,
typically running 2–3× faster than base R and about 1.7× faster than
dqrng (a modern, high-performance RNG package) for uniform, normal, and
exponential draws.

For more information, including implementation details, benchmarking
results, and documentation of engines and distributions, see the main
project readme file on github/jonasson2/randompack. The same page also
links to DEVELOPMENT.md, which contains setup and development
instructions, including details specific to the R interface.

## Cross-platform consistency

Given the same engine and seed, samples obtained on different platforms
(programming language/computer/compiler/OS/architecture) agree. For
uniform, normal, exponential, and integer distributions the agreement is
bit-exact (x == y holds). For the remaining distributions, samples agree
to within ca. 2 ulp. If the `bitexact` parameter is set to `TRUE` the
agreement is bit-exact for all distributions.

## Usage

### Installation, setup, and seeding

``` r
install.packages("randompack")       # From CRAN
library(randompack)                  #

rng <- randompack_rng()              # default engine (x256++simd)
rng <- randompack_rng("pcg64")       # specified engine; rng is randomized by default
randompack_engines()                 # list available engines
rng$seed(123)                        # deterministic seed
rng$seed(123, spawn_key=c(1,2))      # independent substreams
rng$randomize()                      # seed from system entropy
rng2 <- rng$duplicate()              # identical independent copy
```

### Continuous distributions

``` r
x <- rng$unif(100)            # 100 draws from U(0,1)
y <- rng$unif(100, 2, 5)      # 100 draws from U(2,5)
s <- rng$unif(1)              # scalar draw
z <- rng$normal(5)            # 5 standard normal draws
t <- rng$normal(5, 2, 3)      # 5 draws from N(2,3)
u <- rng$beta(50, 2, 5)       # 50 draws from the Beta(2,5) distribution
```

### Discrete distributions

``` r
x <- rng$int(100, 1, 6)       # integers in [1,6]
p <- rng$perm(10)             # permutation of 1:10
s <- rng$sample(20, 5)        # 5-element sample from 1:20
```

### Multivariate normal

``` r
Sigma <- matrix(c(1.0, 0.2, 0.2, 2.0), 2, 2)
X <- rng$mvn(100, Sigma)                       # zero mean
Y <- rng$mvn(50, Sigma, mu=c(1.0, 2.0))        # specified mean
```

### State control and serialization

``` r
rngx <- randompack_rng("x256**")
rngq <- randompack_rng("pcg64")
rngs <- randompack_rng("sfc64")
rngp <- randompack_rng("philox")
rngc <- randompack_rng("chacha20")
rngz <- randompack_rng("squares")
rngr <- randompack_rng("ranlux++")
rngx$set_state(c(1, 2, 3, 4))                  # general state setter
rngq$pcg64_set_inc(c(3, 0, 5, 0))              # change PCG stream increment
rngs$set_state(c(1, 0, 2, 0, 3, 0, 17, 0))     # set full sfc64 state
rngs$sfc64_set_abc(c(7, 0, 11, 0, 13, 0))      # update only a, b, c
rngc$chacha_set_nonce(c(7, 11, 13))            # change ChaCha20 nonce
rngx$jump(128)                                 # jump the state by 2^128 steps
rngp$set_state(c(1, 0, 2, 0, 3, 0, 4, 0, 0, 0, 0, 0)) # set full Philox state
rngp$philox_set_key(c(4, 6))                   # set Philox key
rngz$set_state(c(3, 0, 0, 0))                  # set full Squares state
rngz$squares_set_key(c(4, 0))                  # set Squares key
rngr$jump(32)                                  # jump the state by 2^32 steps

rngy <- randompack_rng("x256**")               # engines must match
state <- rngx$serialize()                      # copy engine state of rngx
rngy$deserialize(state)                        # and put in rngy

full_mantissa(rng, TRUE)                       # enable full 53-bit mantissa (52 bit is default)
rng <- randompack_rng(bitexact=TRUE)           # make agreement across platforms exact
```
