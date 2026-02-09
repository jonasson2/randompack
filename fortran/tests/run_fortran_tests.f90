module testutil
  use, intrinsic :: ieee_arithmetic, only: ieee_is_nan, ieee_is_finite
  implicit none
  private
  public :: fail, assert, approx_equal, approx_equal_d


contains
  subroutine fail(msg)
    character(len=*), intent(in) :: msg
    write(*,'(A)') "FAIL: "//trim(msg)
    stop 1
  end subroutine fail

  subroutine assert(cond, msg)
    logical, intent(in) :: cond
    character(len=*), intent(in) :: msg
    if (.not. cond) call fail(msg)
  end subroutine assert

  pure elemental logical function approx_equal(x, y) result(ok)
    real, intent(in) :: x, y
    ok = abs(x - y) <= epsilon(x)*max(1.0, abs(x), abs(y))
  end function approx_equal

  pure elemental logical function approx_equal_d(x, y) result(ok)
    double precision, intent(in) :: x, y
    ok = abs(x - y) <= epsilon(x)*max(1.0d0, abs(x), abs(y))
  end function approx_equal_d
end module testutil

program test_randompack_fortran
use, intrinsic :: iso_c_binding, only: c_int32_t, c_int64_t, c_int8_t
use, intrinsic :: ieee_arithmetic, only: ieee_is_finite
  use randompack
  use testutil
  implicit none
  type(randompack_rng) :: r1, r2, r3
  type(randompack_rng) :: s1, s2
  type(randompack_rng) :: p1, p2
  type(randompack_philox_ctr) :: ctr
  type(randompack_philox_key) :: key
  character(len=:), allocatable :: names(:), desc(:)
  character(len=:), allocatable :: engine
  logical :: has_pcg, has_x, has_philox, has_squares
  double precision :: x(100), y(100), z(100)
  double precision :: a(7,11)
  real :: xf(100)
  real :: af(7,11)
  integer(c_int32_t) :: iv(50), im(6,9)
  integer(c_int64_t) :: iv64(50)
  integer(c_int8_t), allocatable :: bytes(:)

  call engines(names, desc)
  call assert(size(names) == size(desc), "engines: names/desc size mismatch")
  call assert(size(names) > 0, "engines: empty engine list")

  has_pcg = has_name(names, "pcg64")
  has_x = has_name(names, "x256++simd")
  has_philox = has_name(names, "philox")
  has_squares = has_name(names, "squares")

  if (has_pcg) then
    engine = "pcg64"
  else if (has_x) then
    engine = "x256++simd"
  else
    engine = names(1)
  end if

  write(*,'(A)') "Using engine: "//engine

  !------------------------------------------------------------
  ! Smoke: create + seed + u01 vec/mat
  call r1%create(engine)
  call r1%seed(123_c_int32_t)
  call r1%u01(x)
  call assert(all(0 <= x .and. x < 1 .and. ieee_is_finite(x)), "u01")
  call r1%u01(a)
  call assert(all(0 <= a .and. a < 1 .and. ieee_is_finite(a)), "u01")

  ! unif(a,b) vec/mat
  call r1%seed(123_c_int32_t)
  call r1%unif(x, -1d0, 2d0)
  call assert(all(-1 <= x .and. x <= 2 .and. ieee_is_finite(x)), "unif")
  call r1%unif(a, -3d0, -2d0)
  call assert(all(-3 <= a .and. a <= -2 .and. ieee_is_finite(a)), "unif")

  ! normal vec/mat
  call r1%seed(123_c_int32_t)
  call r1%normal(x, 0d0, 1d0)
  call assert(all(ieee_is_finite(x)), "normal")
  call r1%normal(a, 2d0, 3d0)
  call assert(all(ieee_is_finite(a)), "normal")

  ! u01/unif/normal vec/mat (float)
  call r1%seed(123_c_int32_t)
  call r1%u01(xf)
  call assert(all(0 <= xf .and. xf < 1 .and. ieee_is_finite(xf)), "u01f")
  call r1%u01(af)
  call assert(all(0 <= af .and. af < 1 .and. ieee_is_finite(af)), "u01f")
  call r1%unif(xf, -1., 2.)
  call assert(all(-1 <= xf .and. xf <= 2 .and. ieee_is_finite(xf)), "uniff")
  call r1%unif(af, -3., -2.)
  call assert(all(-3 <= af .and. af <= -2 .and. ieee_is_finite(af)), "uniff")
  call r1%normal(xf, 0., 1.)
  call assert(all(ieee_is_finite(xf)), "normalf")
  call r1%normal(af, 2., 3.)
  call assert(all(ieee_is_finite(af)), "normalf")

  ! beta vec/mat (bounds [0,1])
  call r1%seed(123_c_int32_t)
  call r1%beta(x, 2d0, 3d0)
  call assert(all(0 <= x .and. x <= 1 .and. ieee_is_finite(x)), "beta")
  call r1%beta(a, 2d0, 3d0)
  call assert(all(0 <= a .and. a <= 1 .and. ieee_is_finite(a)), "beta")

  ! other continuous vec
  call r1%seed(123_c_int32_t)
  call r1%lognormal(x, 0d0, 1d0)
  call assert(all(x >= 0 .and. ieee_is_finite(x)), "lognormal")
  call r1%exp(x, 1d0)
  call assert(all(x >= 0 .and. ieee_is_finite(x)), "exp")
  call r1%gamma(x, 2d0, 1d0)
  call assert(all(x >= 0 .and. ieee_is_finite(x)), "gamma")
  call r1%chi2(x, 5d0)
  call assert(all(x >= 0 .and. ieee_is_finite(x)), "chi2")
  call r1%t(x, 5d0)
  call assert(all(ieee_is_finite(x)), "t")
  call r1%f(x, 5d0, 7d0)
  call assert(all(x >= 0 .and. ieee_is_finite(x)), "f")
  call r1%gumbel(x, 0d0, 1d0)
  call assert(all(ieee_is_finite(x)), "gumbel")
  call r1%pareto(x, 1d0, 2d0)
  call assert(all(x >= 0 .and. ieee_is_finite(x)), "pareto")
  call r1%weibull(x, 2d0, 1d0)
  call assert(all(x >= 0 .and. ieee_is_finite(x)), "weibull")
  call r1%skew_normal(x, 0d0, 1d0, 2d0)
  call assert(all(ieee_is_finite(x)), "skew_normal")

  ! other continuous vec (float)
  call r1%seed(123_c_int32_t)
  call r1%lognormal(xf, 0., 1.)
  call assert(all(xf >= 0 .and. ieee_is_finite(xf)), "lognormalf")
  call r1%exp(xf, 1.)
  call assert(all(xf >= 0 .and. ieee_is_finite(xf)), "expf")
  call r1%gamma(xf, 2., 1.)
  call assert(all(xf >= 0 .and. ieee_is_finite(xf)), "gammaf")
  call r1%chi2(xf, 5.)
  call assert(all(xf >= 0 .and. ieee_is_finite(xf)), "chi2f")
  call r1%t(xf, 5.)
  call assert(all(ieee_is_finite(xf)), "tf")
  call r1%f(xf, 5., 7.)
  call assert(all(xf >= 0 .and. ieee_is_finite(xf)), "ff")
  call r1%gumbel(xf, 0., 1.)
  call assert(all(ieee_is_finite(xf)), "gumbelf")
  call r1%pareto(xf, 1., 2.)
  call assert(all(xf >= 0 .and. ieee_is_finite(xf)), "paretof")
  call r1%weibull(xf, 2., 1.)
  call assert(all(xf >= 0 .and. ieee_is_finite(xf)), "weibullf")
  call r1%skew_normal(xf, 0., 1., 2.)
  call assert(all(ieee_is_finite(xf)), "skew_normalf")

  ! int vec/mat
  call r1%seed(123_c_int32_t)
  call r1%int(iv, -2_c_int32_t, 3_c_int32_t)
  call assert(all(-2 <= iv .and. iv <= 3), "int32")
  call r1%int(im, -4_c_int32_t, -1_c_int32_t)
  call assert(all(-4 <= im .and. im <= -1), "int32")
  call r1%int(iv64, -2_c_int64_t, 3_c_int64_t)
  call assert(all(-2 <= iv64 .and. iv64 <= 3), "int64")
  call r1%int(iv64, 1_c_int32_t, 9_c_int32_t)
  call assert(all(1 <= iv64 .and. iv64 <= 9), "int64 bounds32")

  !------------------------------------------------------------
  ! Determinism: seed resets stream
  call r1%seed(777_c_int32_t)
  call r1%u01(x)
  call r1%seed(777_c_int32_t)
  call r1%u01(y)
  call assert(size(x) == size(y) .and. all(approx_equal_d(x, y)), "seed determinism")

  call r1%seed(778_c_int32_t)
  call r1%u01(z)
  call assert(size(x) == size(z) .and. .not. all(approx_equal_d(x, z)), &
    "different seed differs")

  !------------------------------------------------------------
  ! Duplicate: identical then diverge
  call r1%seed(999_c_int32_t)
  call r1%duplicate(r2)
  call r1%u01(x)
  call r2%u01(y)
  call assert(size(x) == size(y) .and. all(approx_equal_d(x, y)), &
    "duplicate initial match")
  call r1%u01(z)    ! advance only r1
  call r1%u01(x)
  call r2%u01(y)
  call assert(size(x) == size(y) .and. .not. all(approx_equal_d(x, y)), &
    "duplicate independence")

  !------------------------------------------------------------
  ! Serialize/deserialize: restore state exactly
  call r1%seed(555_c_int32_t)
  call r1%u01(x)
  call r1%serialize(bytes)
  call r1%u01(y)
  call r3%create(engine)
  call r3%deserialize(bytes)
  call r3%u01(z)
  call assert(size(y) == size(z) .and. all(approx_equal_d(y, z)), &
    "deserialize restores state")

  !------------------------------------------------------------
  ! Optional: squares_set_state repeatability
  if (has_squares) then
    call s1%create("squares")
    call s2%create("squares")
    call s1%squares_set_state(3_c_int64_t, 4_c_int64_t)
    call s2%squares_set_state(3_c_int64_t, 4_c_int64_t)
    call s1%u01(x)
    call s2%u01(y)
    call assert(size(x) == size(y) .and. all(approx_equal_d(x, y)), &
      "squares_set_state repeatability")
    call s1%free()
    call s2%free()
  end if

  ! Optional: philox_set_state repeatability
  if (has_philox) then
    ctr%v = [1_c_int64_t, 2_c_int64_t, 3_c_int64_t, 4_c_int64_t]
    key%v = [5_c_int64_t, 6_c_int64_t]
    call p1%create("philox")
    call p2%create("philox")
    call p1%philox_set_state(ctr, key)
    call p2%philox_set_state(ctr, key)
    call p1%u01(x)
    call p2%u01(y)
    call assert(size(x) == size(y) .and. all(approx_equal_d(x, y)), &
      "philox_set_state repeatability")
    call p1%free()
    call p2%free()
  end if

  call r1%free()
  call r2%free()
  call r3%free()

  write(*,'(A)') "OK: Fortran randompack tests passed."
  stop 0

contains

  logical function has_name(names, target)
    character(len=:), allocatable, intent(in) :: names(:)
    character(len=*), intent(in) :: target
    integer :: i
    has_name = .false.
    do i = 1, size(names)
      if (trim(names(i)) == target) then
        has_name = .true.
        return
      end if
    end do
  end function has_name

end program test_randompack_fortran
