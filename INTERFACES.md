# Language Interfaces

Interfaces to the Randompack C library have been written in Fortran, Python, R,
and Julia. In addition the main header file, randompack.h, contains an `extern
"C"` guard allowing use from C++. The language interfaces are in the following
subdirectories:

    fortran
    python
    r-package
    Randompack.jl
    
The file DEVELOPMENT.md contains information on these interfaces: how to set
them up for local use, build them and install them, how to run tests and
benchmarks, and how to create new releases on PyPI, CRAN, Julia General
Registry, and conda-forge.

## Cross-Language Benchmarks

These benchmark programs and scripts compare Randompack and its language 
interfaces with other libraries and built-in random facilities:

    BENCHMARK                               LIBRARY
    benchmark/TimeDistCpp.cpp               C++ standard library
    benchmark/TimeDistMKL.c                 oneMKL VSL library
    python/examples/TimeDist.py             NumPy continuous distributions
    python/examples/TimeIntegers.py         NumPy discrete sampling
    r-package/inst/examples/TimeDist.R      R random-numbers, R base and dqrng
    Randompack.jl/examples/TimeDist.jl      Julia Random continuous distributions
    Randompack.jl/examples/TimeIntegers.jl  Julia Random discrete sampling
    
(the first two need compilation/building via meson).

## Cross platform consistency

Given the same engine and seed, samples obtained with the language packages
agree with each other and with the C library. For U(0,1), N(0,1), Exp(1), and
discrete distributions the agreement is bit-identical (x == y holds). For the
remaining distributions, samples agree to within about 2 ulp. All interfaces
include a `bitexact` parameter which makes the agreement bit-identical for all
distributions.
