# randompack

This package provides Python bindings to the C library Randompack, a random number
generation toolkit that also includes interfaces for Julia, R, and Fortran.
Randompack exposes a collection of modern RNG engines, including xoshiro256++/**,
PCG64 DXSM, sfc64, Philox, and ChaCha20, together with a range of probability
distributions, both integer and continuous. The library allows matching random
draws across platforms and supported language interfaces. It provides unbounded
and bounded integer draws, permutations, sampling without replacement, and 14
continuous distributions, ranging from basic ones (uniform, normal, exponential),
through commonly used distributions (beta, gamma), to more specialized ones
(such as skew-normal). Multivariate normal sampling is also supported.

Through SIMD instructions on modern CPUs, the inherently fast default engine
xoshiro256++ delivers high throughput for bulk generation, typically providing
3–6× faster performance than NumPy for uniform, normal, and exponential draws.

For more information, including implementation details, benchmarking results, and
documentation of engines and distributions, see the main project readme file at
https://github.com/jonasson2/randompack.

## Usage

### Installation, setup, and seeding
```sh
pip install randompack
```

```python
import randompack as rp

rng = rp.Rng()                         # default engine (x256++simd)
rng = rp.Rng("pcg64")                  # specified engine; rng is randomized by default
rp.engines()                           # list available engines
rng.seed(123)                          # deterministic seed
rng.seed(123, spawn_key=[1, 2])        # independent substreams
rng.randomize()                        # seed from system entropy
rng2 = rng.duplicate()                 # identical independent copy
```

### Continuous distributions
```python
x = rng.unif(100)                       # 100 draws from U(0,1)
y = rng.unif(100, a=2, b=5)             # 100 draws from U(2,5)
s = rng.unif()                          # scalar draw (length-1 array)
z = rng.normal(5)                       # 5 standard normal draws
t = rng.normal(5, mu=2, sigma=3)        # 5 draws from N(2,3)
u = rng.beta(50, a=2, b=5)              # 50 draws from the Beta(2,5) distribution
```

### Discrete distributions
```python
x = rng.int(100, 1, 6)                  # integers in [1,6]
p = rng.perm(10)                        # permutation of 1..10
s = rng.sample(20, 5)                   # 5-element sample from 1..20
```

### Multivariate normal
```python
import numpy as np

Sigma = np.array([[1.0, 0.2], [0.2, 2.0]])
X = rng.mvn(100, Sigma)                         # zero mean
Y = rng.mvn(50, Sigma, mu=np.array([1.0, 2.0])) # specified mean
Z = np.zeros((100, 2))
rng.mvn(out=Z, cov=Sigma, mean=np.array([1.0, 2.0]))
```

### State control and serialization
```python
rngx = rp.Rng("x256**")
rngp = rp.Rng("philox")
rp.set_state(rngx, state=[1, 2, 3, 4])          # general state setter
rp.philox_set_state(rngp, ctr=[1, 2, 3, 4], key=[4, 6])

rngy = rp.Rng("x256**")
state = rp.serialize(rngx)
rp.deserialize(rngy, state)

rp.full_mantissa(rng, True)                     # enable full 53-bit mantissa (52 bit is default)
```
