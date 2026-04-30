program check_fortran_ints
use, intrinsic :: iso_fortran_env, only: int32, int64
use, intrinsic :: iso_c_binding, only: c_int32_t, c_int64_t
use randompack
implicit none
type(randompack_rng) :: rng
integer :: idflt(3)
integer(int32) :: i32(3)
integer(int64) :: i64(3)
integer(c_int32_t) :: ic32(3)
integer(c_int64_t) :: ic64(3)
character(len=*), parameter :: hdr = 'TYPE                 KIND  DRAWS'
character(len=*), parameter :: fmt = '(A, T22, I4, T28, 3(I0,1X))'
call rng%create()
call rng%seed(123_int32)
call rng%int(idflt, 1, 9)
call rng%int(i32, 1_int32, 9_int32)
call rng%int(i64, 1_int64, 9_int64)
call rng%int(ic32, 1, 9)
call rng%int(ic64, 1, 9)
write(*,'(A)') hdr
write(*,fmt) 'default integer', kind(idflt), idflt
write(*,fmt) 'integer(int32)', kind(i32), i32
write(*,fmt) 'integer(int64)', kind(i64), i64
write(*,fmt) 'integer(c_int32_t)', kind(ic32), ic32
write(*,fmt) 'integer(c_int64_t)', kind(ic64), ic64
call rng%free()
end program check_fortran_ints
