# Randompack Fortran Interface

This package provides Fortran bindings to the C library Randompack, a random
number generation toolkit that also includes interfaces for Python, R, and
Julia. Randompack exposes a collection of modern RNG engines, including
xoshiro256++/**, PCG64 DXSM, sfc64, ranlux++, Philox, and ChaCha20, together
with a range of probability distributions, both integer and continuous. The
library allows matching random draws across platforms and supported language
interfaces. It provides unbounded and bounded integer draws, raw integer
bitstreams, random permutations, sampling without replacement, and 14
continuous distributions, ranging from basic ones (uniform, normal,
exponential), through commonly used distributions (beta, gamma), to more
specialized ones (such as skew-normal). Multivariate normal sampling is also
supported.

Through SIMD instructions on modern CPUs, the inherently fast default engine
xoshiro256++ delivers high throughput for bulk generation, typically providing
3-6 times faster performance than NumPy for uniform, normal, and exponential
draws.

For more information, including implementation details, benchmarking results,
and documentation of engines and distributions, see the main project readme file
at https://github.com/jonasson2/randompack. The same page also links to
DEVELOPMENT.md, which contains setup and development instructions, including
details specific to the Fortran interface.

## Cross platform consistency

Given the same engine and seed, samples obtained on different platforms
(programming language/computer/compiler/OS/architecture) agree. For uniform,
normal, exponential, and integer distributions the agreement is bit-exact (x ==
y holds). For the remaining distributions, samples agree to within ca. 2 ulp. If
the `bitexact` parameter is set to `.true.` the agreement is bit-exact for all
distributions.

## Usage

### Installation, setup, and seeding
```sh
meson setup build --buildtype=release
meson compile -C build
meson install -C build
```

```fortran
program demo_randompack
  use randompack
  implicit none
  character(len=:), allocatable :: names(:), desc(:)
  type(randompack_rng) :: rng, rng2
  double precision :: x(100)
  call rng%create()                         ! default engine (x256++simd)
  call rng%create('pcg64')                  ! specified engine; randomized
  call rng%create('pcg64', bitexact=.true.) ! make samples bit-identical
  call rng%create(full_mantissa=.true.)     ! enable full 53-bit mantissa
  call engines(names, desc)                 ! list available engines
  call rng%seed(123)                        ! deterministic seed
  call rng%seed(123, [1, 2])                ! independent substreams
  call rng%randomize()                      ! seed from system entropy
  call rng%duplicate(rng2)                  ! identical independent copy
  call rng%unif(x)                          ! simple draw into x
end program
```

The supported build path is via Meson/Ninja. The Fortran module is built from
`fortran/src/randompack.f90` and installed together with the shared C library.

### Continuous distributions

```fortran
program demo_continuous
  use randompack
  implicit none
  type(randompack_rng) :: rng
  double precision :: x(100), y(100), s(1), z(5), t(5), u(50)
  real :: v(5)
  call rng%create()
  call rng%unif(x)              ! 100 float64 draws from U(0,1)
  call rng%unif(y, 2d0, 5d0)    ! 100 draws from U(2,5)
  call rng%unif(s)              ! scalar draw in a length-1 array
  call rng%normal(z)            ! 5 standard normal draws
  call rng%normal(t, 2d0, 3d0)  ! 5 draws from N(2,3)
  call rng%beta(u, 2d0, 5d0)    ! 50 draws from the Beta(2,5) distribution
  call rng%normal(v)            ! single precision
  call rng%unif(x)              ! use shape and data type of x
end program
```

### Discrete distributions

```fortran
program demo_discrete
  use, intrinsic :: iso_c_binding, only: c_int32_t, c_int64_t
  use randompack
  implicit none
  type(randompack_rng) :: rng
  integer(c_int32_t) :: i32(100), p32(10), s32(5), raw32(1000)
  integer(c_int64_t) :: i64(100), raw64(1000)
  call rng%create()
  call rng%int(i32, 1, 6)  ! integers in [1,6] (inclusive)
  call rng%int(i64, 1, 6)  ! 64-bit integers in [1,6]
  call rng%perm(p32)       ! permutation of 1...10
  call rng%sample(s32, 20) ! 5-element sample from 1...20 (without replacement)
  call rng%raw(raw32)      ! raw 32-bit bitstream values
  call rng%raw(raw64)      ! raw 64-bit bitstream values
end program
```

### Multivariate normal

```fortran
program demo_mvn
  use randompack
  implicit none
  type(randompack_rng) :: rng
  double precision :: Sigma(2,2), X(100,2), Y(50,2), Z(2,50), mu(2)
  Sigma = reshape([1d0, 0.2d0, 0.2d0, 2d0], [2,2])
  mu = [1d0, 2d0]
  call rng%create()
  call rng%mvn(X, Sigma)                ! zero mean, X is n x d
  call rng%mvn(Y, Sigma, mu)            ! specified mean
  call rng%mvn(Z, Sigma, mu, trans='T') ! transposed layout, d x n
end program
```

### State control and serialization

```fortran
program demo_state
  use, intrinsic :: iso_c_binding, only: c_int8_t
  use randompack
  implicit none
  integer(c_int8_t), allocatable :: state(:)
  type(randompack_rng) :: rng, rngc, rngp, rngq, rngr
  type(randompack_rng) :: rngs, rngw, rngx, rngy, rngz
  call rngc%create('chacha20')
  call rngp%create('philox')
  call rngq%create('pcg64')
  call rngr%create('ranlux++')
  call rngs%create('sfc64')
  call rngw%create('cwg128')
  call rngx%create('x256**')
  call rngy%create('x256**')
  call rngz%create('squares')
      
  call rngq%pcg64_set_inc([3, 5])       ! change PCG stream increment
  call rngq%advance(2**16)              ! advance pcg64 by 2^16 steps
  call rngq%advance([2**16, 0])         ! also advances by 2^16 steps
  call rngq%jump(16)                    ! and also advances by 2^16 steps
  call rngr%jump(32)                    ! jump ranlux++ state by 2^32 steps
  call rngx%jump(128)                   ! jump x256** state by 2^128 steps
  call rngc%chacha_set_nonce([7,11,13]) ! change ChaCha20 nonce, each entry<2^32
  call rngw%cwg128_set_weyl([3, 5])     ! change CWG128 Weyl increment
  call rngs%sfc64_set_abc([7, 11, 13])  ! change a, b, c
  call rngp%philox_set_key([4, 6])      ! change Philox key
  call rngz%squares_set_key(4)          ! change squares key
  call rngx%set_state([1, 2, 3, 4])     ! general state setter

  call rngx%serialize(state)            ! copy engine state of rngx
  call rngy%deserialize(state)          ! and put in rngy (engines must match)

  call rng%create(bitexact=.true.)            ! make agreement exact
  call rng%create('philox', bitexact=.true.)  ! bitexact with specified engine
  call rng%create(full_mantissa=.true.)       ! enable full 53-bit mantissa
                                              ! (52-bit is default)
end program
```

### Notes on types

The floating-point drawing routines are generic over `double precision` and
default `real` arrays, and they accept both rank-1 and rank-2 output arrays.
There is no scalar-returning API, so a scalar draw should use a length-1 array.

The integer API is also generic. `rng%int` dispatches to either 32-bit or 64-bit
integer draws depending on the output array type. `rng%perm` and `rng%sample`
also support both 32-bit and 64-bit integer output vectors, and they return
1-based indices. `rng%raw` fills 32-bit or 64-bit integer arrays with raw random
bitstream values.

For deterministic seeding, `seed` accepts either 32-bit or 64-bit integers. The
64-bit variant must fit in the signed 32-bit range and is converted internally.
`spawn_key` is optional and must match the seed type.

Multivariate normal sampling is currently provided for `double precision`
matrices only. The default is `trans='N'`, with `X(n,d)`; `trans='T'` uses
`X(d,n)`. `Sigma` must be square, and `mu`, when present, must have length `d`.

## Errors

On failure, the Fortran interface raises `error stop` with the underlying error
message from the C library when available. `last_error()` returns the most
recent C-side error string for the current RNG instance.
