pure function to_c_string(s) result(cs)
  character(len=*), intent(in) :: s
  character(kind=c_char), allocatable :: cs(:)
  integer :: i, n
  n = len_trim(s)
  allocate(cs(n+1))
  do i = 1, n
    cs(i) = transfer(s(i:i), cs(i))
  end do
  cs(n+1) = c_null_char
end function

function cstr_to_fstring(p) result(msg)
  type(c_ptr), intent(in) :: p
  character(len=:), allocatable :: msg
  character(kind=c_char), pointer :: s(:)
  integer :: i, n
  if (.not. c_associated(p)) then
    msg = ""
    return
  end if
  call c_f_pointer(p, s, [MAXSTRLEN])
  n = 0
  do i = 1, size(s)
    if (s(i) == c_null_char) exit
    n = n + 1
  end do
  allocate(character(len=n) :: msg)
  do i = 1, n
    msg(i:i) = transfer(s(i), msg(i:i))
  end do
end function

subroutine check_kinds()
  logical, save :: done = .false.
  if (done) return
  if (kind(1.0d0) /= c_double) then
    error stop "randompack: c_double mismatch"
  end if
  if (kind(1.0) /= c_float) then
    error stop "randompack: c_float mismatch"
  end if
  done = .true.
end subroutine

function rp_error_message(self, where) result(msg)
  class(randompack_rng), intent(in) :: self
  character(len=*), intent(in) :: where
  character(len=:), allocatable :: msg
  character(len=:), allocatable :: emsg
  type(c_ptr) :: p
  if (.not. c_associated(self%p)) then
    msg = where//": rng not created"
    return
  end if
  p = crp_last_error(self%p)
  emsg = cstr_to_fstring(p)
  if (len(emsg) == 0) then
    msg = where//": randompack call failed"
  else
    msg = where//": "//trim(emsg)
  end if
end function

subroutine engines(names, descriptions)
  character(len=:), allocatable, intent(out) :: names(:)
  character(len=:), allocatable, intent(out) :: descriptions(:)
  integer(c_int) :: neng, eng_maxlen, desc_maxlen
  character(kind=c_char), allocatable, target :: raw_eng(:)
  character(kind=c_char), allocatable, target :: raw_desc(:)
  logical(c_bool) :: ok
  integer :: i, j, k, n, m
  call check_kinds()
  neng = 0_c_int
  eng_maxlen = 0_c_int
  desc_maxlen = 0_c_int
  ok = crp_engines(c_null_ptr, c_null_ptr, neng, eng_maxlen, desc_maxlen)
  if (.not. ok) error stop "engines: query failed"
  n = int(neng)
  if (n <= 0) then
    allocate(character(len=0) :: names(0))
    allocate(character(len=0) :: descriptions(0))
    return
  end if
  allocate(raw_eng(n*int(eng_maxlen)))
  allocate(raw_desc(n*int(desc_maxlen)))
  ok = crp_engines(c_loc(raw_eng(1)), c_loc(raw_desc(1)), neng, eng_maxlen, desc_maxlen)
  if (.not. ok) error stop "engines: fill failed"
  allocate(character(len=int(eng_maxlen)) :: names(n))
  allocate(character(len=int(desc_maxlen)) :: descriptions(n))
  do i = 1, n
    m = 0
    do j = 1, int(eng_maxlen)
      k = (i-1)*int(eng_maxlen) + j
      if (raw_eng(k) == c_null_char) exit
      m = m + 1
    end do
    names(i) = ""
    do j = 1, m
      k = (i-1)*int(eng_maxlen) + j
      names(i)(j:j) = transfer(raw_eng(k), names(i)(j:j))
    end do
    m = 0
    do j = 1, int(desc_maxlen)
      k = (i-1)*int(desc_maxlen) + j
      if (raw_desc(k) == c_null_char) exit
      m = m + 1
    end do
    descriptions(i) = ""
    do j = 1, m
      k = (i-1)*int(desc_maxlen) + j
      descriptions(i)(j:j) = transfer(raw_desc(k), descriptions(i)(j:j))
    end do
  end do
end subroutine
