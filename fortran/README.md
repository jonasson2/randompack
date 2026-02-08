# Randompack Fortran Interface

Randompack provides a Fortran 2003+ interface to the Randompack C library. It exposes
modern RNG engines and a wide range of distributions, with deterministic streams across
platforms and languages.

## Requirements

- A Fortran compiler with `iso_c_binding` support.
- The Randompack C library (`librandompack`) built and available to link.
- 64-bit platform (the underlying C library requires it).
- The interface uses explicit integer kinds (`c_int32_t`/`c_int64_t`) and is
  compatible with -i8 / -fdefault-integer-8.

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
use, intrinsic :: iso_c_binding, only: c_int32_t
use randompack
implicit none
type(randompack_rng) :: rng
double precision :: x(100)
real :: xf(100)
integer(c_int32_t) :: iv(10)

call rng%create()      ! default engine (x256++simd)
call rng%seed(123_c_int32_t)     ! deterministic seed

call rng%u01(x)        ! U(0,1) in Float64
call rng%normal(x, 0.0d0, 1.0d0)
call rng%normal(xf, 0.0, 1.0)

call rng%int(iv, 1_c_int32_t, 6_c_int32_t) ! integers in [1,6]
call rng%free()
end program
```

### Notes on types

The interface uses standard Fortran types:

- Double precision real for Float64 draws
- Default real (single precision) for Float32 draws
- `integer(c_int32_t)` or `integer(c_int64_t)` for integer draws
- `integer(c_int64_t)` for engine-specific state setters
- `integer(c_int8_t)` buffers for serialize/deserialize

At runtime it checks that `double precision` matches C `double` and default
`real` matches C `float`.

## Engines

Query available engines and descriptions:

```fortran
character(len=:), allocatable :: names(:), desc(:)
call engines(names, desc)
```

Returned names/descriptions are trimmed strings (no trailing NUL).

Create with a specific engine:

```fortran
call rng%create("pcg64")
```

## RNG management

The `rng` type provides:

- `create([engine])`
- `free`
- `duplicate(out)`
- `randomize` (seed from system entropy)
- `full_mantissa(enable)` (logical)
- `seed(seed[, spawn_key])`
- `serialize(buf)` / `deserialize(buf)`
- `set_state(state)`
- `philox_set_state(ctr, key)`
- `squares_set_state(ctr, key)`
- `last_error()`

### Seeding

`seed` is a 32-bit integer seed (`c_int32_t`); `spawn_key` (optional) is a
`c_int32_t` vector that derives independent substreams.

## Distributions

All routines accept rank-1 or rank-2 arrays; use a length-1 array for scalar draws.

### Continuous (Float64 / Float32)

Each of the following is a generic that accepts `double precision` or default
`real` arrays, with the usual parameters:

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
double precision :: a(100), b(10,10)
call rng%unif(a, -1.0d0, 2.0d0)
call rng%skew_normal(b, 0.0d0, 1.0d0, 2.0d0)
```

### Discrete (integers)

`int` draws uniform integers in the inclusive range `[m,n]`:

```fortran
integer(c_int32_t) :: iv(50)
integer(c_int64_t) :: i64(50)
call rng%int(iv, -2_c_int32_t, 3_c_int32_t)
call rng%int(i64, 1_c_int64_t, 10_c_int64_t)
```

## State control and serialization

Serialize to a byte buffer (portable representation):

```fortran
integer(c_int8_t), allocatable :: buf(:)
call rng%serialize(buf)
call rng%deserialize(buf)
```

Engine-specific state setters:

```fortran
type(randompack_philox_ctr) :: ctr
type(randompack_philox_key) :: key
ctr%v = [1_c_int64_t, 2_c_int64_t, 3_c_int64_t, 4_c_int64_t]
key%v = [5_c_int64_t, 6_c_int64_t]
call rng%philox_set_state(ctr, key)
call rng%squares_set_state(1_c_int64_t, 2_c_int64_t)
```

## Errors

On failure, the Fortran interface raises `error stop` with the underlying error message
from the C library (when available). Most procedures accept an optional `ok`
argument; if present and a call fails, `ok` is set to `.false.` and no `error stop`
is raised.
`last_error()` returns the most recent C-side error string for this RNG instance.

## References

- Main project README: `../README.md`
- Julia interface: `../Randompack.jl/README.md`
- R interface: `../r-package/README.md`
