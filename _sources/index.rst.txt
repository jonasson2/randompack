Randompack
==========

Randompack is a high-performance random number generation library
designed for correctness, reproducibility, and cross-language
consistency. It provides modern random number generators together
with a wide range of commonly used probability distributions.

The Python interface is built on a C11 core shared with the C,
Fortran, and R bindings. Under the same engine and seed, streams
are compatible across languages.

Quick example
-------------

.. code-block:: python

    import randompack
    rng = randompack.Rng()
    rng.seed(42)
    x = rng.normal(5)

Reproducibility and substreams
------------------------------

Randompack emphasizes deterministic reproducibility. Generators
may be seeded explicitly and support reproducible construction
of independent substreams via optional spawn keys.

.. code-block:: python

    import numpy as np, randompack, threading

    results = [None, None]

    def worker(i):
        rng = randompack.Rng()
        rng.seed(123, spawn_key=[i])
        results[i] = rng.normal(1000)

    t0 = threading.Thread(target=worker, args=(0,))
    t1 = threading.Thread(target=worker, args=(1,))
    t0.start(); t1.start()
    t0.join();  t1.join()

    X = np.array(results)   # shape (2, 1000)

Engines
-------

Randompack provides several modern random number generators
("engines"). ChaCha20 is available as a cryptographically secure
generator. Philox and Squares are counter-based generators; the
remaining engines are state-based.

Available engines include:

- ``x256++``   (xoshiro256++)
- ``x256**``   (xoshiro256**)
- ``xoro++``   (xoroshiro128++)
- ``x128+``    (xorshift128+)
- ``pcg64``    (PCG64 DXSM)
- ``sfc64``    (sfc64)
- ``philox``   (Philox-4×64)
- ``squares``  (Squares64)
- ``chacha20`` (ChaCha20)

Engine names are case-insensitive.

Distributions
-------------

The following distributions are available:

- Uniform (continuous)
- Normal
- Lognormal
- Exponential
- Gamma
- Beta
- Chi-square
- Student’s t
- F
- Weibull
- Gumbel
- Pareto
- Skew normal
- Multivariate normal (float64)

In addition, Randompack provides:

- Raw bit streams
- Bounded and interval integers
- Random permutations
- Sampling without replacement

Continuous distributions are available in both float64 and float32
precision (unless otherwise noted).

Algorithms
----------

Distribution algorithms are implemented directly from their
standard published descriptions.

- Normal and exponential: Ziggurat method (Marsaglia & Tsang, 2000)
- Gamma: Marsaglia–Tsang method
- Beta: ratio of independent gamma variates
- Chi-square: Gamma(ν/2, 2)
- Student’s t: Normal divided by scaled Gamma
- F: ratio of scaled Gamma variates
- Weibull: power transform of exponential
- Gumbel: double logarithm transform of uniform
- Pareto: exponential transform
- Skew normal: Azzalini construction
- Multivariate normal: Cholesky / pivoted Cholesky factorization
- Permutation: Fisher–Yates shuffle
- Sampling without replacement: Floyd’s algorithm and reservoir sampling
- Integer intervals: Lemire method

Verification and testing
------------------------

Correctness of the underlying generators is a central design goal.
Raw bit streams are compared against authoritative upstream
reference implementations where available.

The test suite checks API behavior and distributional correctness,
including support containment, statistical balance tests, and
cross-language reproducibility.

External statistical validation via TestU01 and PractRand is supported.

API reference
-------------

The complete API reference is generated from the Python
docstrings and is available below.

.. toctree::
   :maxdepth: 2

   reference/index
