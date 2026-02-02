# Randompack Fortran Interface

Randompack provides a Fortran 2003+ interface to the Randompack C library. It exposes
modern RNG engines and a wide range of distributions, with deterministic streams across
platforms and languages.

## Requirements

- A Fortran compiler with `iso_c_binding` support.
- The Randompack C library (`librandompack`) built and available to link.
- 64-bit platform (the underlying C library requires it).

## Build (Meson)

The top-level build includes the Fortran interface when `build_fortran` is enabled (or
auto-detected):

```
meson setup build -Dbuild_fortran=enabled
ninja -C build
```

The Fortran module is built from `fortran/src/randompack.f90`. You can also compile it
manually, but the Meson build is the supported path.

## Quick start

```fortran
program demo_randompack
use, intrinsic :: iso_c_binding, only: c_double, c_float, c_int
use randompack
implicit none
type(rng) :: r
real(rp_double) :: x(100)
real(c_float) :: xf(100)
integer(rp_int) :: iv(10)

call r%create()        ! default engine (x256++simd)
call r%seed(123)       ! deterministic seed

call r%u01(x)          ! U(0,1) in Float64
call r%normal(x, 0.0d0, 1.0d0)
call r%normal(xf, 0.0_c_float, 1.0_c_float)

call r%int(iv, 1_c_int, 6_c_int)   ! integers in [1,6]
call r%free()
end program
```

### Notes on types

The module exposes kind parameters for convenience:

- `rp_double` (C double)
- `rp_int` (C int)
- `rp_i64` (C int64)
- `rp_i8` (C int8)

Use these kinds for portability. Continuous draws accept `real(rp_double)` and
`real(c_float)` arrays; integer draws use `integer(rp_int)` arrays.

## Engines

Query available engines and descriptions:

```fortran
character(len=:), allocatable :: names(:), desc(:)
call engines(names, desc)
```

Create with a specific engine:

```fortran
call r%create("pcg64")
```

## RNG management

The `rng` type provides:

- `create([engine])`
- `free`
- `duplicate`
- `randomize` (seed from system entropy)
- `full_mantissa(enable)` (logical)
- `seed(seed[, spawn_key])`
- `serialize(buf)` / `deserialize(buf)`
- `set_state(state)`
- `philox_set_state(ctr, key)`
- `squares_set_state(ctr, key)`
- `last_error()`

### Seeding

`seed` is converted to a C `int` (32-bit). Optional `spawn_key` is an integer vector used
to derive independent substreams.

## Distributions

The interface provides vector and matrix draws (rank-1 or rank-2 arrays). There are no
scalar-specific methods; use a length‑1 array for a single draw.

### Continuous (Float64 / Float32)

Each of the following is a generic that accepts `real(rp_double)` or `real(c_float)`
arrays (vector or matrix), with the usual parameters:

- `u01` (U(0,1))
- `unif` (a, b)
- `norm` (standard normal)
- `normal` (mu, sigma)
- `exp` (scale)
- `lognormal` (mu, sigma)
- `gamma` (shape, scale)
- `beta` (a, b)
- `chi2` (nu)
- `t` (nu)
- `f` (nu1, nu2)
- `gumbel` (mu, beta)
- `pareto` (xm, alpha)
- `weibull` (shape, scale)
- `skew_normal` (mu, sigma, alpha)

Example:

```fortran
real(rp_double) :: a(100), b(10,10)
call r%unif(a, -1.0d0, 2.0d0)
call r%skew_normal(b, 0.0d0, 1.0d0, 2.0d0)
```

### Discrete (integers)

`int` draws uniform integers in the inclusive range `[m,n]`:

```fortran
integer(rp_int) :: iv(50), im(6,9)
call r%int(iv, -2_c_int, 3_c_int)
call r%int(im, 1_c_int, 10_c_int)
```

## State control and serialization

Serialize to a byte buffer:

```fortran
integer(rp_i8), allocatable :: buf(:)
call r%serialize(buf)
call r%deserialize(buf)
```

Engine-specific state setters:

```fortran
type(randompack_philox_ctr) :: ctr
type(randompack_philox_key) :: key
ctr%v = [1_rp_i64, 2_rp_i64, 3_rp_i64, 4_rp_i64]
key%v = [5_rp_i64, 6_rp_i64]
call r%philox_set_state(ctr, key)
call r%squares_set_state(1_rp_i64, 2_rp_i64)
```

## Errors

On failure, the Fortran interface raises `error stop` with the underlying error message
from the C library (when available).

## References

- Main project README: `../README.md`
- Julia interface: `../Randompack.jl/README.md`
- R interface: `../r-package/README.md`
