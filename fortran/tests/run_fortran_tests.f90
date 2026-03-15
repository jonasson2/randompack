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
use, intrinsic :: ieee_arithmetic, only: ieee_is_finite, ieee_set_flag, ieee_underflow
  use randompack
  use testutil
  implicit none
  type(randompack_rng) :: r1, r2, r3
  type(randompack_rng) :: s1, s2
  type(randompack_rng) :: p1, p2
  character(len=:), allocatable :: names(:), desc(:)
  character(len=:), allocatable :: engine
  logical :: has_pcg, has_x, has_philox, has_squares
  double precision :: x(100), y(100), z(100)
  double precision :: a(7,11)
  double precision :: xe(1), xn(1), xl(1)
  double precision :: xe1(1), xn1(1), xl1(1)
  real :: xf(100)
  real :: af(7,11)
  integer(c_int32_t) :: iv(50), im(6,9)
  integer(c_int64_t) :: iv64(50)
  integer(c_int64_t) :: inc_pcg(2)
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
  ! Smoke: create + seed + unif vec/mat
  call r1%create(engine)
  call r1%seed(123)
  call r1%unif(x)
  call assert(all(0 <= x .and. x < 1 .and. ieee_is_finite(x)), "unif")
  call r1%unif(a)
  call assert(all(0 <= a .and. a < 1 .and. ieee_is_finite(a)), "unif")

  ! unif(a,b) vec/mat
  call r1%seed(123)
  call r1%unif(x, -1d0, 2d0)
  call assert(all(-1 <= x .and. x <= 2 .and. ieee_is_finite(x)), "unif")
  call r1%unif(a, -3d0, -2d0)
  call assert(all(-3 <= a .and. a <= -2 .and. ieee_is_finite(a)), "unif")

  ! normal vec/mat
  call r1%seed(123)
  call r1%normal(x)
  call assert(all(ieee_is_finite(x)), "normal")
  call r1%normal(a, 2d0, 3d0)
  call assert(all(ieee_is_finite(a)), "normal")

  ! unif/normal vec/mat (float)
  call r1%seed(123)
  call r1%unif(xf)
  call assert(all(0 <= xf .and. xf < 1 .and. ieee_is_finite(xf)), "unif")
  call r1%unif(af)
  call assert(all(0 <= af .and. af < 1 .and. ieee_is_finite(af)), "unif")
  call r1%unif(xf, -1., 2.)
  call assert(all(-1 <= xf .and. xf <= 2 .and. ieee_is_finite(xf)), "uniff")
  call r1%unif(af, -3., -2.)
  call assert(all(-3 <= af .and. af <= -2 .and. ieee_is_finite(af)), "uniff")
  call r1%normal(xf, 0., 1.)
  call assert(all(ieee_is_finite(xf)), "normalf")
  call r1%normal(af, 2., 3.)
  call assert(all(ieee_is_finite(af)), "normalf")

  ! beta vec/mat (bounds [0,1])
  call r1%seed(123)
  call r1%beta(x, 2d0, 3d0)
  call assert(all(0 <= x .and. x <= 1 .and. ieee_is_finite(x)), "beta")
  call r1%beta(a, 2d0, 3d0)
  call assert(all(0 <= a .and. a <= 1 .and. ieee_is_finite(a)), "beta")

  ! other continuous vec
  call r1%seed(123)
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

  ! defaults for distributions with optional parameters
  call r1%seed(123)
  call r1%exp(x)
  call assert(all(x >= 0 .and. ieee_is_finite(x)), "exp default")
  call r1%lognormal(x)
  call assert(all(x >= 0 .and. ieee_is_finite(x)), "lognormal default")
  call r1%gamma(x, 2d0)
  call assert(all(x >= 0 .and. ieee_is_finite(x)), "gamma default scale")
  call r1%gumbel(x)
  call assert(all(ieee_is_finite(x)), "gumbel default")
  call r1%weibull(x, 2d0)
  call assert(all(x >= 0 .and. ieee_is_finite(x)), "weibull default scale")
  call r1%skew_normal(x, alpha=2d0)
  call assert(all(ieee_is_finite(x)), "skew_normal default")
  call r1%seed(42)
  call r1%exp(xe)
  call r1%normal(xn)
  call r1%lognormal(xl)
  call r1%seed(42)
  call r1%exp(xe1, 1d0)
  call r1%normal(xn1, 0d0, 1d0)
  call r1%lognormal(xl1, 0d0, 1d0)
  call assert(approx_equal_d(xe(1), xe1(1)), "exp default matches explicit")
  call assert(approx_equal_d(xn(1), xn1(1)), "normal default matches explicit")
  call assert(approx_equal_d(xl(1), xl1(1)), "lognormal default matches explicit")

  ! other continuous vec (float)
  call r1%seed(123)
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
  call r1%seed(123)
  call r1%int(iv, -2, 3)
  call assert(all(-2 <= iv .and. iv <= 3), "int32")
  call r1%int(im, -4, -1)
  call assert(all(-4 <= im .and. im <= -1), "int32")
  call r1%int(iv64, -2, 3)
  call assert(all(-2 <= iv64 .and. iv64 <= 3), "int64")
  call r1%int(iv64, 1, 9)
  call assert(all(1 <= iv64 .and. iv64 <= 9), "int64 bounds32")

  !------------------------------------------------------------
  ! Determinism: seed resets stream
  call r1%seed(777)
  call r1%unif(x)
  call r1%seed(777)
  call r1%unif(y)
  call assert(size(x) == size(y) .and. all(approx_equal_d(x, y)), "seed determinism")

  call r1%seed(778)
  call r1%unif(z)
  call assert(size(x) == size(z) .and. .not. all(approx_equal_d(x, z)), &
    "different seed differs")

  !------------------------------------------------------------
  ! Jump: advance stream (xor-family only)
  if (has_x) then
    call r2%create("x256++simd")
    call r3%create("x256++simd")
    call r2%seed(123)
    call r3%seed(123)
    call r3%jump(128)
    call r2%unif(xe)
    call r3%unif(xn)
    call assert(.not. approx_equal_d(xe(1), xn(1)), "jump advances stream")
    call r2%free()
    call r3%free()
  end if

  !------------------------------------------------------------
  ! Duplicate: identical then diverge
  call r1%seed(999)
  call r1%duplicate(r2)
  call r1%unif(x)
  call r2%unif(y)
  call assert(size(x) == size(y) .and. all(approx_equal_d(x, y)), &
    "duplicate initial match")
  call r1%unif(z)    ! advance only r1
  call r1%unif(x)
  call r2%unif(y)
  call assert(size(x) == size(y) .and. .not. all(approx_equal_d(x, y)), &
    "duplicate independence")

  !------------------------------------------------------------
  ! Serialize/deserialize: restore state exactly
  call r1%seed(555)
  call r1%unif(x)
  call r1%serialize(bytes)
  call r1%unif(y)
  call r3%create(engine)
  call r3%deserialize(bytes)
  call r3%unif(z)
  call assert(size(y) == size(z) .and. all(approx_equal_d(y, z)), &
    "deserialize restores state")

  !------------------------------------------------------------
  ! Optional: squares_set_state repeatability
  if (has_squares) then
    call s1%create("squares")
    call s2%create("squares")
    call s1%squares_set_state(3, 4)
    call s2%squares_set_state(3, 4)
    call s1%unif(x)
    call s2%unif(y)
    call assert(size(x) == size(y) .and. all(approx_equal_d(x, y)), &
      "squares_set_state repeatability")
    call s1%free()
    call s2%free()
  end if

  ! Optional: philox_set_state repeatability
  if (has_philox) then
    call p1%create("philox")
    call p2%create("philox")
    call p1%philox_set_state([1_c_int64_t, 2_c_int64_t, 3_c_int64_t, 4_c_int64_t], &
      [5_c_int64_t, 6_c_int64_t])
    call p2%philox_set_state([1_c_int64_t, 2_c_int64_t, 3_c_int64_t, 4_c_int64_t], &
      [5_c_int64_t, 6_c_int64_t])
    call p1%unif(x)
    call p2%unif(y)
    call assert(size(x) == size(y) .and. all(approx_equal_d(x, y)), &
      "philox_set_state repeatability")
    call p1%free()
    call p2%free()
  end if

  if (has_pcg) then
    call p1%create("pcg64")
    call p1%set_state([1_c_int64_t, 0_c_int64_t, 1_c_int64_t, 0_c_int64_t])
    inc_pcg = [3_c_int64_t, 5_c_int64_t]
    call p1%pcg64_set_inc(inc_pcg)
    call p1%unif(xe)
    call assert(all(0 <= xe .and. xe < 1 .and. ieee_is_finite(xe)), &
      "pcg64_set_inc")
    call p1%free()
  end if

  call p1%create("sfc64")
  call p2%create("sfc64")
  call p1%sfc64_set_state([7_c_int64_t, 11_c_int64_t, 13_c_int64_t], 17_c_int64_t)
  call p2%sfc64_set_state([7_c_int64_t, 11_c_int64_t, 13_c_int64_t], 17_c_int64_t)
  call p1%unif(x)
  call p2%unif(y)
  call assert(size(x) == size(y) .and. all(approx_equal_d(x, y)), &
    "sfc64_set_state repeatability")
  call p1%free()
  call p2%free()

  call r1%free()
  call r2%free()
  call r3%free()

  call ieee_set_flag(ieee_underflow, .false.)
  write(*,'(A)') "OK: Fortran randompack tests passed."

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
