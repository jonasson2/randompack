# randompack

This package provides Python bindings to the C library Randompack, a random number
generation toolkit that also includes interfaces for Julia, R, and Fortran. Randompack
exposes a collection of modern RNG engines, including xoshiro256++/**, PCG64 DXSM, sfc64,
ranlux++, Philox, and ChaCha20, together with a range of probability distributions, both
integer and continuous. The library allows matching random draws across platforms and
supported language interfaces. It provides unbounded and bounded integer draws,
permutations, sampling without replacement, and 14 continuous distributions, ranging from
basic ones (uniform, normal, exponential), through commonly used distributions (beta,
gamma), to more specialized ones (such as skew-normal). Multivariate normal sampling is
also supported.

Through SIMD instructions on modern CPUs, the inherently fast default engine xoshiro256++
delivers high throughput for bulk generation, typically providing 3–6× faster performance
than NumPy for uniform, normal, and exponential draws.

For more information, including implementation details, benchmarking results, and
documentation of engines and distributions, see the main project readme file at
https://github.com/jonasson2/randompack. The same page also links to DEVELOPMENT.md, which
contains setup and development instructions, including details specific to the Python
interface.

## Cross platform consistency

Given the same engine and seed, samples obtained on different platforms (programming
language/computer/compiler/OS/architecture) agree. For uniform, normal, exponential, and
integer distributions the agreement is bit-exact (x == y holds). For the remaining
distributions, samples agree to within ca. 2 ulp. If the `bitexact` parameter is set to
`true` the agreement is bit-exact for all distributions.

## Usage

### Installation, setup, and seeding
```sh
pip install randompack
```

```python
import numpy as np
import randompack
rng = randompack.Rng()              # default engine (x256++simd)
rng = randompack.Rng("pcg64")       # specified engine; rng is randomized by default
randompack.engines()                # list available engines
rng.seed(123)                       # deterministic seed
rng.seed(123, spawn_key=[1, 2])     # independent substreams
rng.randomize()                     # seed from system entropy
rng2 = rng.duplicate()              # identical independent copy
```

### Continuous distributions
```python
x = rng.unif(100)                   # 100 float64 draws from U(0,1)
y = rng.unif(100, a=2, b=5)         # 100 draws from U(2,5)
s = rng.unif()                      # scalar draw
z = rng.normal(5)                   # 5 standard normal draws
t = rng.normal(5, mu=2, sigma=3)    # 5 draws from N(2,3)
u = rng.beta(50, a=2, b=5)          # 50 draws from the Beta(2,5) distribution
v = rng.normal(5, dtype=np.float32) # single precision
rng.unif(out=x)                     # use shape and data type of x
```

### Discrete distributions
```python
x = rng.int(100, 1, 6)              # integers in [1,6] (inclusive)
p = rng.perm(10)                    # permutation of 0...9
s = rng.sample(20, 5)               # 5-element sample from 0...19 (without replacement)
b = rng.raw(1000)                   # bytes object with 1000 elements
```

### Multivariate normal
```python
Sigma = np.array([[1.0, 0.2], [0.2, 2.0]])
X = rng.mvn(100, Sigma)                          # zero mean
Y = rng.mvn(50, Sigma, mu=np.array([1.0, 2.0]))  # specified mean
Z = np.zeros((100, 2))                           # 2 columns
rng.mvn(Sigma, out=Z)                            # Sigma must be 2×2
```

### State control and serialization
```python
rngx = randompack.Rng("x256**")
rngp = randompack.Rng("philox")
rngq = randompack.Rng("pcg64")
rngs = randompack.Rng("sfc64")
rngc = randompack.Rng("chacha20")
rngz = randompack.Rng("squares")
rngr = randompack.Rng("ranlux++")
rngx.set_state(state=[1,2,3,4])                  # general state setter
rngx.jump(128)                                   # jump the state by 2^128 steps
rngp.set_state([1,2,3,4,0,0])                    # set full Philox state
rngp.philox_set_key([4,6])                       # set Philox key
rngq.pcg64_set_inc([3, 5])                       # change PCG stream increment
rngs.set_state([1, 2, 3, 17])                    # set full sfc64 state
rngs.sfc64_set_abc([7, 11, 13])                  # update only a, b, c
rngc.chacha_set_nonce([7, 11, 13])               # change ChaCha20 nonce
rngz.set_state([3, 0])                           # set full Squares state
rngz.squares_set_key(4)                          # set Squares key
rngr.jump(32)                                    # jump the state by 2^32 steps

rngy = randompack.Rng("x256**")  # engines must match
state = rngx.serialize()         # copy engine state of rngx
rngy.deserialize(state)          # and put in rngy

rng.full_mantissa(True)          # enable full 53-bit mantissa (52-bit is default)
rng = randompack.Rng(bitexact=True)            # make agreement across platforms exact
rng = randompack.Rng("philox", bitexact=True)  # exact agreement with specified engine
```
