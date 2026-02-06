module testutil
use, intrinsic :: iso_fortran_env, only: int64
use, intrinsic :: ieee_arithmetic, only: ieee_is_nan, ieee_is_finite
implicit none
private
public :: fail, assert_true, assert_all_finite, assert_all_in_01, &
  assert_all_in_ab, assert_all_int_in_mn, assert_equal_r8, assert_equal_i4, &
  assert_equal_i8, assert_not_equal_r8

interface assert_all_finite
  module procedure assert_all_finite_r8
  module procedure assert_all_finite_r8m
end interface

interface assert_all_in_01
  module procedure assert_all_in_01_r8
  module procedure assert_all_in_01_r8m
end interface

interface assert_all_in_ab
  module procedure assert_all_in_ab_r8
  module procedure assert_all_in_ab_r8m
end interface

interface assert_all_int_in_mn
  module procedure assert_all_int_in_mn_i4
  module procedure assert_all_int_in_mn_i4m
end interface

contains
subroutine fail(msg)
  character(len=*), intent(in) :: msg
  write(*,'(A)') "FAIL: "//trim(msg)
  stop 1
end subroutine
subroutine assert_true(cond, msg)
  logical, intent(in) :: cond
  character(len=*), intent(in) :: msg
  if (.not. cond) call fail(msg)
end subroutine
subroutine assert_all_finite_r8(x, msg)
  double precision, intent(in) :: x(:)
  character(len=*), intent(in) :: msg
  if (any(ieee_is_nan(x))) call fail(msg//" (NaN)")
  if (.not. all(ieee_is_finite(x))) call fail(msg//" (Inf)")
end subroutine
subroutine assert_all_finite_r8m(x, msg)
  double precision, intent(in) :: x(:,:)
  character(len=*), intent(in) :: msg
  if (any(ieee_is_nan(x))) call fail(msg//" (NaN)")
  if (.not. all(ieee_is_finite(x))) call fail(msg//" (Inf)")
end subroutine
subroutine assert_all_in_01_r8(x, msg)
  double precision, intent(in) :: x(:)
  character(len=*), intent(in) :: msg
  call assert_all_finite(x, msg)
  if (.not. all(x >= 0.0d0)) call fail(msg//" (x<0)")
  if (.not. all(x <  1.0d0)) call fail(msg//" (x>=1)")
end subroutine
subroutine assert_all_in_01_r8m(x, msg)
  double precision, intent(in) :: x(:,:)
  character(len=*), intent(in) :: msg
  call assert_all_finite(x, msg)
  if (.not. all(x >= 0.0d0)) call fail(msg//" (x<0)")
  if (.not. all(x <  1.0d0)) call fail(msg//" (x>=1)")
end subroutine
subroutine assert_all_in_ab_r8(x, a, b, msg)
  double precision, intent(in) :: x(:)
  double precision, intent(in) :: a, b
  character(len=*), intent(in) :: msg
  call assert_all_finite(x, msg)
  if (.not. all(x >= a)) call fail(msg//" (x<a)")
  if (.not. all(x <= b)) call fail(msg//" (x>b)")
end subroutine
subroutine assert_all_in_ab_r8m(x, a, b, msg)
  double precision, intent(in) :: x(:,:)
  double precision, intent(in) :: a, b
  character(len=*), intent(in) :: msg
  call assert_all_finite(x, msg)
  if (.not. all(x >= a)) call fail(msg//" (x<a)")
  if (.not. all(x <= b)) call fail(msg//" (x>b)")
end subroutine
subroutine assert_all_int_in_mn_i4(x, m, n, msg)
  integer, intent(in) :: x(:)
  integer, intent(in) :: m, n
  character(len=*), intent(in) :: msg
  if (.not. all(x >= m)) call fail(msg//" (x<m)")
  if (.not. all(x <= n)) call fail(msg//" (x>n)")
end subroutine
subroutine assert_all_int_in_mn_i4m(x, m, n, msg)
  integer, intent(in) :: x(:,:)
  integer, intent(in) :: m, n
  character(len=*), intent(in) :: msg
  if (.not. all(x >= m)) call fail(msg//" (x<m)")
  if (.not. all(x <= n)) call fail(msg//" (x>n)")
end subroutine
subroutine assert_equal_r8(a, b, msg)
  double precision, intent(in) :: a(:), b(:)
  character(len=*), intent(in) :: msg
  if (size(a) /= size(b)) call fail(msg//" (size)")
  if (maxval(abs(a - b)) > 0.0d0) call fail(msg//" (values)")
end subroutine
subroutine assert_not_equal_r8(a, b, msg)
  double precision, intent(in) :: a(:), b(:)
  character(len=*), intent(in) :: msg
  if (size(a) /= size(b)) call fail(msg//" (size)")
  if (maxval(abs(a - b)) <= 0.0d0) call fail(msg//" (expected difference)")
end subroutine
subroutine assert_equal_i4(a, b, msg)
  integer, intent(in) :: a(:), b(:)
  character(len=*), intent(in) :: msg
  if (size(a) /= size(b)) call fail(msg//" (size)")
  if (.not. all(a == b)) call fail(msg//" (values)")
end subroutine
subroutine assert_equal_i8(a, b, msg)
  integer, intent(in) :: a(:), b(:)
  character(len=*), intent(in) :: msg
  if (size(a) /= size(b)) call fail(msg//" (size)")
  if (.not. all(a == b)) call fail(msg//" (values)")
end subroutine
end module testutil

program test_randompack_fortran
use, intrinsic :: iso_fortran_env, only: int64
use randompack
use testutil
implicit none
type(rng) :: r1, r2, r3
type(rng) :: s1, s2
type(rng) :: p1, p2
type(randompack_philox_ctr) :: ctr
type(randompack_philox_key) :: key
character(len=:), allocatable :: names(:), desc(:)
character(len=:), allocatable :: engine
logical :: has_pcg, has_x, has_philox, has_squares
double precision :: x(100), y(100), z(100)
double precision :: a(7,11)
integer :: iv(50), im(6,9)
integer, allocatable :: bytes(:)

call engines(names, desc)
call assert_true(size(names) == size(desc), "engines: names/desc size mismatch")
call assert_true(size(names) > 0, "engines: empty engine list")

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
call r1%seed(123)
call r1%u01(x)
call assert_all_in_01(x, "u01 vec")
call r1%u01(a)
call assert_all_in_01(a, "u01 mat")

! unif(a,b) vec/mat
call r1%seed(123)
call r1%unif(x, -1.0d0, 2.0d0)
call assert_all_in_ab(x, -1.0d0, 2.0d0, "unif vec")
call r1%unif(a, -3.0d0, -2.0d0)
call assert_all_in_ab(a, -3.0d0, -2.0d0, "unif mat")

! normal vec/mat
call r1%seed(123)
call r1%normal(x, 0.0d0, 1.0d0)
call assert_all_finite(x, "normal vec")
call r1%normal(a, 2.0d0, 3.0d0)
call assert_all_finite(a, "normal mat")

! beta vec/mat (bounds [0,1])
call r1%seed(123)
call r1%beta(x, 2.0d0, 3.0d0)
call assert_all_finite(x, "beta vec")
call assert_true(all(x >= 0.0d0) .and. all(x <= 1.0d0), "beta vec bounds")
call r1%beta(a, 2.0d0, 3.0d0)
call assert_all_finite(a, "beta mat")
call assert_true(all(a >= 0.0d0) .and. all(a <= 1.0d0), "beta mat bounds")

! int vec/mat
call r1%seed(123)
call r1%int(iv, -2, 3)
call assert_all_int_in_mn(iv, -2, 3, "int vec")
call r1%int(im, -4, -1)
call assert_all_int_in_mn(im, -4, -1, "int mat")

!------------------------------------------------------------
! Determinism: seed resets stream
call r1%seed(777)
call r1%u01(x)
call r1%seed(777)
call r1%u01(y)
call assert_equal_r8(x, y, "seed determinism")

call r1%seed(778)
call r1%u01(z)
call assert_not_equal_r8(x, z, "different seed differs")

!------------------------------------------------------------
! Duplicate: identical then diverge
call r1%seed(999)
call r1%duplicate(r2)
call r1%u01(x)
call r2%u01(y)
call assert_equal_r8(x, y, "duplicate initial match")
call r1%u01(z)    ! advance only r1
call r1%u01(x)
call r2%u01(y)
call assert_not_equal_r8(x, y, "duplicate independence")

!------------------------------------------------------------
! Serialize/deserialize: restore state exactly
call r1%seed(555)
call r1%u01(x)
call r1%serialize(bytes)
call r1%u01(y)
call r3%create(engine)
call r3%deserialize(bytes)
call r3%u01(z)
call assert_equal_r8(y, z, "serialize/deserialize restores state")

!------------------------------------------------------------
! Optional: squares_set_state repeatability
if (has_squares) then
  call s1%create("squares")
  call s2%create("squares")
  call s1%squares_set_state(3_int64, 4_int64)
  call s2%squares_set_state(3_int64, 4_int64)
  call s1%u01(x)
  call s2%u01(y)
  call assert_equal_r8(x, y, "squares_set_state repeatability")
  call s1%free()
  call s2%free()
end if

! Optional: philox_set_state repeatability
if (has_philox) then
  ctr%v = [1_int64, 2_int64, 3_int64, 4_int64]
  key%v = [5_int64, 6_int64]
  call p1%create("philox")
  call p2%create("philox")
  call p1%philox_set_state(ctr, key)
  call p2%philox_set_state(ctr, key)
  call p1%u01(x)
  call p2%u01(y)
  call assert_equal_r8(x, y, "philox_set_state repeatability")
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
end function

end program test_randompack_fortran
