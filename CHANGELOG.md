# Changelog

## [0.1.5] - 2026-04-26

Availability: in preparation.

- Speed up normal draws on Windows.
- Fix Windows compiler warnings and portability issues for MSVC, clang, and icx.
- Prepare Julia/Yggdrasil packaging for Randompack_jll submission.

## [0.1.4] - 2026-04-23

Availability: tagged on GitHub; not yet uploaded to PyPI or CRAN.

- Packaging and documentation updates to support conda-forge submission.
- Cleanup and simplification of the Fortran interface sources and Meson build
  integration.
- Added Fortran support for `perm`, `sample`, and `mvn`.
- Updated the Fortran README.md to match the Python/R/Julia readme files.

## [0.1.3] - 2026-04-22

Availability: accepted by CRAN; not uploaded to PyPI.

- Technical release focused on CRAN submission rather than new user-facing
  functionality.
- Harmonized RNG control APIs across bindings, including constructor-level
  `full_mantissa` support and the move from `pcg64_advance` to `advance`.
- Refined state-control support and tests across the Python, R, Julia, and
  Fortran interfaces.
- Updated and aligned the Python, R, and Julia README files to match the
  revised cross-language APIs and examples.
- Updated R build configuration, package metadata, and documentation for CRAN.

## [0.1.2] - 2026-04-21

Availability: PyPI.

- Substantial update to the core library and Python bindings.
- Expanded engine and stream-control support, including `ranlux++`, jump
  support, and additional engine-specific state/key/nonce/increment controls.
- Improved low-level portability and performance infrastructure with broader
  SIMD handling and revised BLAS/LAPACK support.
- Extended examples, benchmarks, and documentation, including broader coverage
  of state management and supported distributions.

## [0.1.1] - 2026-02-23

Availability: PyPI.

- Initial public Python release of Randompack.
- Python bindings for the C core library with multiple RNG engines, integer and
  continuous distributions, and reproducible cross-platform streams.
