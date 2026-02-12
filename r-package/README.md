# randompack: Fast Random Number Generation for R

Randompack provides fast, high-quality random number generation for R with
multiple modern RNG engines and a wide variety of distributions. The package is
built on a portable C library that emphasizes correctness, reproducibility, and
performance.

## Installation

From CRAN (once published):

        install.packages("randompack")

Development version from GitHub:

        # install.packages("remotes")
        remotes::install_github("jonasson2/randompack", subdir = "r-package")

## Quick Start

        library(randompack)

        # Create an RNG with the default engine (x256++simd)
        rng <- randompack_rng()

        # Generate random variates
        x <- rng$normal(100, mu = 0, sigma = 1)
        y <- rng$unif(50, a = 0, b = 1)
        z <- rng$int(20, min = 1, max = 100)

        # Use a different engine
        rng_pcg <- randompack_rng("pcg64")

        # Reproducible results
        rng$seed(12345)

## Available Engines

Randompack supports multiple high-quality RNG engines including xoshiro256++/**,
PCG64, Philox-4×64, ChaCha20, and others. Use `randompack_engines()` to see the
complete list.

## Features

- **Multiple engines**: Choose from modern, high-performance generators
- **Wide distribution coverage**: Continuous, discrete, and multivariate distributions
- **Reproducibility**: Seeding and state serialization
- **Full mantissa option**: Toggle 53-bit mantissas for double-precision draws
- **Parallel streams**: Independent substreams via spawn keys
- **Thread-safe**: Multiple independent RNG objects

## Documentation

- [Package documentation](https://CRAN.R-project.org/package=randompack) (once on CRAN)
- See `?randompack_rng` for detailed usage
- [C library documentation](../README.md) for implementation details

## About the C Library

The R package is a binding to the randompack C library, which provides the same
functionality across multiple languages (C, Fortran, R, Python) with compatible
streams. For details on the underlying implementation, verification, testing,
and benchmarks, see the [main project README](../README.md).

## License

MIT License - see [LICENSE](LICENSE) file
