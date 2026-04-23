submodule (randompack) randompack_impl
implicit none
contains
!-*- fortran -*-
module procedure to_c_string
  integer :: i, n
  n = len_trim(s)
  allocate(cs(n+1))
  do i = 1, n
    cs(i) = transfer(s(i:i), cs(i))
  end do
  cs(n+1) = c_null_char
end procedure

module procedure cstr_to_fstring
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
end procedure

module procedure check_kinds
  logical, save :: done = .false.
  if (done) return
  if (kind(1.0d0) /= c_double) then
    error stop "randompack: c_double mismatch"
  end if
  if (kind(1.0) /= c_float) then
    error stop "randompack: c_float mismatch"
  end if
  done = .true.
end procedure

module procedure rp_error_message
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
end procedure

module procedure engines
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
end procedure

module procedure rp_finalize
  call self%free()
end procedure

module procedure rp_free
  if (c_associated(self%p)) then
    call crp_free(self%p)
    self%p = c_null_ptr
  end if
end procedure

module procedure rp_create
  character(kind=c_char), allocatable, target :: eng(:)
  character(len=:), allocatable :: emsg
  type(c_ptr) :: p
  logical(c_bool) :: c_ok
  call check_kinds()
  if (c_associated(self%p)) call self%free()
  if (present(engine)) then
    eng = to_c_string(engine)
    self%p = crp_create(c_loc(eng(1)))
  else
    self%p = crp_create(c_null_ptr)
  end if
  if (.not. c_associated(self%p)) then
    error stop "create: failed"
  end if
  p = crp_last_error(self%p)
  emsg = cstr_to_fstring(p)
  if (len(emsg) > 0) then
    error stop "create: "//trim(emsg)
  end if
  if (present(bitexact)) then
    if (bitexact) then
      c_ok = crp_bitexact(self%p, .true._c_bool)
      if (.not. c_ok) error stop rp_error_message(self, "bitexact")
    end if
  end if
  if (present(full_mantissa)) then
    if (full_mantissa) then
      c_ok = crp_full_mantissa(self%p, .true._c_bool)
      if (.not. c_ok) error stop rp_error_message(self, "full_mantissa")
    end if
  end if
end procedure

module procedure rp_duplicate
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "duplicate")
  out%p = crp_duplicate(self%p)
  if (.not. c_associated(out%p)) error stop rp_error_message(self, "duplicate")
end procedure

module procedure rp_randomize
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "randomize")
  c_ok = crp_randomize(self%p)
  if (.not. c_ok) error stop rp_error_message(self, "randomize")
end procedure

module procedure rp_jump
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "jump")
  c_ok = crp_jump(p, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "jump")
end procedure

module procedure rp_seed32
  type(c_ptr) :: keyp
  integer(c_int) :: nkey
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "seed")
  if (present(spawn_key)) then
    keyp = c_loc(spawn_key(1))
    nkey = int(size(spawn_key), c_int)
  else
    keyp = c_null_ptr
    nkey = 0_c_int
  end if
  c_ok = crp_seed(seed, keyp, nkey, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "seed")
end procedure

module procedure rp_seed64
  integer(c_int32_t) :: seed32
  integer(c_int32_t), allocatable, target :: key32(:)
  integer(c_int64_t) :: min32, max32
  type(c_ptr) :: keyp
  integer(c_int) :: nkey
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "seed")
  max32 = int(huge(0_c_int32_t), c_int64_t)
  min32 = -max32 - 1_c_int64_t
  if (seed < min32 .or. seed > max32) then
    error stop "seed: out of range"
  end if
  seed32 = int(seed, c_int32_t)
  if (present(spawn_key)) then
    if (any(spawn_key < min32 .or. spawn_key > max32)) then
      error stop "seed: spawn_key out of range"
    end if
    allocate(key32(size(spawn_key)))
    key32 = int(spawn_key, c_int32_t)
    keyp = c_loc(key32(1))
    nkey = int(size(key32), c_int)
  else
    keyp = c_null_ptr
    nkey = 0_c_int
  end if
  c_ok = crp_seed(seed32, keyp, nkey, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "seed")
end procedure

module procedure unif_vec
  double precision :: aa, bb
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "unif")
  aa = 0
  bb = 1
  if (present(a)) aa = a
  if (present(b)) bb = b
  if (.not. (aa < bb)) error stop "unif: require a < b"
  c_ok = crp_unif(x, int(size(x), c_size_t), aa, bb, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "unif")
end procedure

module procedure unif_mat
  double precision :: aa, bb
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "unif")
  aa = 0
  bb = 1
  if (present(a)) aa = a
  if (present(b)) bb = b
  if (.not. (aa < bb)) error stop "unif: require a < b"
  c_ok = crp_unif(x, int(size(x), c_size_t), aa, bb, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "unif")
end procedure

module procedure uniff_vec
  real :: aa, bb
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "unif")
  aa = 0
  bb = 1
  if (present(a)) aa = a
  if (present(b)) bb = b
  if (.not. (aa < bb)) error stop "unif: require a < b"
  c_ok = crp_uniff(x, int(size(x), c_size_t), aa, bb, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "unif")
end procedure

module procedure uniff_mat
  real :: aa, bb
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "unif")
  aa = 0
  bb = 1
  if (present(a)) aa = a
  if (present(b)) bb = b
  if (.not. (aa < bb)) error stop "unif: require a < b"
  c_ok = crp_uniff(x, int(size(x), c_size_t), aa, bb, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "unif")
end procedure

module procedure normal_vec
  double precision :: mu0, sig0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "normal")
  mu0 = 0
  sig0 = 1
  if (present(mu)) mu0 = mu
  if (present(sigma)) sig0 = sigma
  if (.not. (sig0 > 0)) error stop "normal: sigma must be positive"
  c_ok = crp_normal(x, int(size(x), c_size_t), mu0, sig0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "normal")
end procedure

module procedure normal_mat
  double precision :: mu0, sig0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "normal")
  mu0 = 0
  sig0 = 1
  if (present(mu)) mu0 = mu
  if (present(sigma)) sig0 = sigma
  if (.not. (sig0 > 0)) error stop "normal: sigma must be positive"
  c_ok = crp_normal(x, int(size(x), c_size_t), mu0, sig0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "normal")
end procedure

module procedure normalf_vec
  real :: mu0, sig0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "normal")
  mu0 = 0
  sig0 = 1
  if (present(mu)) mu0 = mu
  if (present(sigma)) sig0 = sigma
  if (.not. (sig0 > 0)) error stop "normal: sigma must be positive"
  c_ok = crp_normalf(x, int(size(x), c_size_t), mu0, sig0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "normal")
end procedure

module procedure normalf_mat
  real :: mu0, sig0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "normal")
  mu0 = 0
  sig0 = 1
  if (present(mu)) mu0 = mu
  if (present(sigma)) sig0 = sigma
  if (.not. (sig0 > 0)) error stop "normal: sigma must be positive"
  c_ok = crp_normalf(x, int(size(x), c_size_t), mu0, sig0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "normal")
end procedure

module procedure exp_vec
  double precision :: s0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "exp")
  s0 = 1
  if (present(scale)) s0 = scale
  if (.not. (s0 > 0)) error stop "exp: scale must be positive"
  c_ok = crp_exp(x, int(size(x), c_size_t), s0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "exp")
end procedure

module procedure exp_mat
  double precision :: s0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "exp")
  s0 = 1
  if (present(scale)) s0 = scale
  if (.not. (s0 > 0)) error stop "exp: scale must be positive"
  c_ok = crp_exp(x, int(size(x), c_size_t), s0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "exp")
end procedure

module procedure expf_vec
  real :: s0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "exp")
  s0 = 1
  if (present(scale)) s0 = scale
  if (.not. (s0 > 0)) error stop "exp: scale must be positive"
  c_ok = crp_expf(x, int(size(x), c_size_t), s0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "exp")
end procedure

module procedure expf_mat
  real :: s0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "exp")
  s0 = 1
  if (present(scale)) s0 = scale
  if (.not. (s0 > 0)) error stop "exp: scale must be positive"
  c_ok = crp_expf(x, int(size(x), c_size_t), s0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "exp")
end procedure

module procedure lognormal_vec
  double precision :: mu0, sig0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "lognormal")
  mu0 = 0
  sig0 = 1
  if (present(mu)) mu0 = mu
  if (present(sigma)) sig0 = sigma
  if (.not. (sig0 > 0)) error stop "lognormal: sigma must be positive"
  c_ok = crp_lognormal(x, int(size(x), c_size_t), mu0, sig0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "lognormal")
end procedure

module procedure lognormal_mat
  double precision :: mu0, sig0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "lognormal")
  mu0 = 0
  sig0 = 1
  if (present(mu)) mu0 = mu
  if (present(sigma)) sig0 = sigma
  if (.not. (sig0 > 0)) error stop "lognormal: sigma must be positive"
  c_ok = crp_lognormal(x, int(size(x), c_size_t), mu0, sig0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "lognormal")
end procedure

module procedure lognormalf_vec
  real :: mu0, sig0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "lognormal")
  mu0 = 0
  sig0 = 1
  if (present(mu)) mu0 = mu
  if (present(sigma)) sig0 = sigma
  if (.not. (sig0 > 0)) error stop "lognormal: sigma must be positive"
  c_ok = crp_lognormalf(x, int(size(x), c_size_t), mu0, sig0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "lognormal")
end procedure

module procedure lognormalf_mat
  real :: mu0, sig0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "lognormal")
  mu0 = 0
  sig0 = 1
  if (present(mu)) mu0 = mu
  if (present(sigma)) sig0 = sigma
  if (.not. (sig0 > 0)) error stop "lognormal: sigma must be positive"
  c_ok = crp_lognormalf(x, int(size(x), c_size_t), mu0, sig0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "lognormal")
end procedure

module procedure gamma_vec
  double precision :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "gamma")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "gamma: shape/scale must be positive"
  c_ok = crp_gamma(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "gamma")
end procedure

module procedure gamma_mat
  double precision :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "gamma")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "gamma: shape/scale must be positive"
  c_ok = crp_gamma(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "gamma")
end procedure

module procedure gammaf_vec
  real :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "gamma")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "gamma: shape/scale must be positive"
  c_ok = crp_gammaf(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "gamma")
end procedure

module procedure gammaf_mat
  real :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "gamma")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "gamma: shape/scale must be positive"
  c_ok = crp_gammaf(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "gamma")
end procedure

module procedure beta_vec
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "beta")
  c_ok = crp_beta(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "beta")
end procedure

module procedure beta_mat
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "beta")
  c_ok = crp_beta(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "beta")
end procedure

module procedure betaf_vec
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "beta")
  c_ok = crp_betaf(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "beta")
end procedure

module procedure betaf_mat
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "beta")
  c_ok = crp_betaf(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "beta")
end procedure

module procedure chi2_vec
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "chi2")
  c_ok = crp_chi2(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "chi2")
end procedure

module procedure chi2_mat
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "chi2")
  c_ok = crp_chi2(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "chi2")
end procedure

module procedure chi2f_vec
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "chi2")
  c_ok = crp_chi2f(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "chi2")
end procedure

module procedure chi2f_mat
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "chi2")
  c_ok = crp_chi2f(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "chi2")
end procedure

module procedure t_vec
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "t")
  c_ok = crp_t(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "t")
end procedure

module procedure t_mat
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "t")
  c_ok = crp_t(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "t")
end procedure

module procedure tf_vec
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "t")
  c_ok = crp_tf(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "t")
end procedure

module procedure tf_mat
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "t")
  c_ok = crp_tf(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "t")
end procedure

module procedure f_vec
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "f")
  c_ok = crp_f(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "f")
end procedure

module procedure f_mat
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "f")
  c_ok = crp_f(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "f")
end procedure

module procedure ff_vec
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "f")
  c_ok = crp_ff(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "f")
end procedure

module procedure ff_mat
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "f")
  c_ok = crp_ff(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "f")
end procedure

module procedure gumbel_vec
  double precision :: mu0, b0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "gumbel")
  mu0 = 0
  b0 = 1
  if (present(mu)) mu0 = mu
  if (present(beta)) b0 = beta
  if (.not. (b0 > 0)) error stop "gumbel: beta must be positive"
  c_ok = crp_gumbel(x, int(size(x), c_size_t), mu0, b0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "gumbel")
end procedure

module procedure gumbel_mat
  double precision :: mu0, b0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "gumbel")
  mu0 = 0
  b0 = 1
  if (present(mu)) mu0 = mu
  if (present(beta)) b0 = beta
  if (.not. (b0 > 0)) error stop "gumbel: beta must be positive"
  c_ok = crp_gumbel(x, int(size(x), c_size_t), mu0, b0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "gumbel")
end procedure

module procedure gumbelf_vec
  real :: mu0, b0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "gumbel")
  mu0 = 0
  b0 = 1
  if (present(mu)) mu0 = mu
  if (present(beta)) b0 = beta
  if (.not. (b0 > 0)) error stop "gumbel: beta must be positive"
  c_ok = crp_gumbelf(x, int(size(x), c_size_t), mu0, b0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "gumbel")
end procedure

module procedure gumbelf_mat
  real :: mu0, b0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "gumbel")
  mu0 = 0
  b0 = 1
  if (present(mu)) mu0 = mu
  if (present(beta)) b0 = beta
  if (.not. (b0 > 0)) error stop "gumbel: beta must be positive"
  c_ok = crp_gumbelf(x, int(size(x), c_size_t), mu0, b0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "gumbel")
end procedure

module procedure pareto_vec
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "pareto")
  c_ok = crp_pareto(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "pareto")
end procedure

module procedure pareto_mat
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "pareto")
  c_ok = crp_pareto(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "pareto")
end procedure

module procedure paretof_vec
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "pareto")
  c_ok = crp_paretof(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "pareto")
end procedure

module procedure paretof_mat
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "pareto")
  c_ok = crp_paretof(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "pareto")
end procedure

module procedure weibull_vec
  double precision :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "weibull")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "weibull: shape/scale must be positive"
  c_ok = crp_weibull(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "weibull")
end procedure

module procedure weibull_mat
  double precision :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "weibull")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "weibull: shape/scale must be positive"
  c_ok = crp_weibull(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "weibull")
end procedure

module procedure weibullf_vec
  real :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "weibull")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "weibull: shape/scale must be positive"
  c_ok = crp_weibullf(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "weibull")
end procedure

module procedure weibullf_mat
  real :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "weibull")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "weibull: shape/scale must be positive"
  c_ok = crp_weibullf(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "weibull")
end procedure

module procedure skew_normal_vec
  double precision :: mu0, sig0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "skew_normal")
  mu0 = 0
  sig0 = 1
  if (present(mu)) mu0 = mu
  if (present(sigma)) sig0 = sigma
  if (.not. (sig0 > 0)) error stop "skew_normal: sigma must be positive"
  c_ok = crp_skew_normal(x, int(size(x), c_size_t), mu0, sig0, alpha, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "skew_normal")
end procedure

module procedure skew_normal_mat
  double precision :: mu0, sig0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "skew_normal")
  mu0 = 0
  sig0 = 1
  if (present(mu)) mu0 = mu
  if (present(sigma)) sig0 = sigma
  if (.not. (sig0 > 0)) error stop "skew_normal: sigma must be positive"
  c_ok = crp_skew_normal(x, int(size(x), c_size_t), mu0, sig0, alpha, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "skew_normal")
end procedure

module procedure skew_normalf_vec
  real :: mu0, sig0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "skew_normal")
  mu0 = 0
  sig0 = 1
  if (present(mu)) mu0 = mu
  if (present(sigma)) sig0 = sigma
  if (.not. (sig0 > 0)) error stop "skew_normal: sigma must be positive"
  c_ok = crp_skew_normalf(x, int(size(x), c_size_t), mu0, sig0, alpha, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "skew_normal")
end procedure

module procedure skew_normalf_mat
  real :: mu0, sig0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "skew_normal")
  mu0 = 0
  sig0 = 1
  if (present(mu)) mu0 = mu
  if (present(sigma)) sig0 = sigma
  if (.not. (sig0 > 0)) error stop "skew_normal: sigma must be positive"
  c_ok = crp_skew_normalf(x, int(size(x), c_size_t), mu0, sig0, alpha, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "skew_normal")
end procedure

module procedure mvn_mat
  double precision, allocatable, target :: sig_tmp(:,:), mu_tmp(:)
  type(c_ptr) :: mu_ptr
  character(kind=c_char), target :: transp
  character(len=1) :: trans1
  integer(c_int) :: d, ldX
  integer(c_size_t) :: n
  logical(c_bool) :: c_ok
  integer :: d1, d2
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "mvn")
  d1 = size(Sigma, 1)
  d2 = size(Sigma, 2)
  if (d1 <= 0 .or. d2 /= d1) then
    error stop "mvn: Sigma must be a non-empty square matrix"
  end if
  allocate(sig_tmp(d1, d1))
  sig_tmp = Sigma
  if (present(mu)) then
    if (size(mu) /= d1) then
      error stop "mvn: mu length must match Sigma"
    end if
    allocate(mu_tmp(d1))
    mu_tmp = mu
    mu_ptr = c_loc(mu_tmp(1))
  else
    mu_ptr = c_null_ptr
  end if
  trans1 = 'N'
  if (present(trans)) trans1 = trans
  if (trans1 >= 'a' .and. trans1 <= 'z') trans1 = achar(iachar(trans1) - 32)
  if (trans1 /= 'N' .and. trans1 /= 'T') then
    error stop "mvn: trans must be 'N' or 'T'"
  end if
  if (trans1 == 'N') then
    if (size(x, 2) /= d1) then
      error stop "mvn: Sigma dimension must match size(x,2) for trans='N'"
    end if
    n = int(size(x, 1), c_size_t)
    ldX = int(size(x, 1), c_int)
  else
    if (size(x, 1) /= d1) then
      error stop "mvn: Sigma dimension must match size(x,1) for trans='T'"
    end if
    n = int(size(x, 2), c_size_t)
    ldX = int(size(x, 1), c_int)
  end if
  transp = trans1
  d = int(d1, c_int)
  c_ok = crp_mvn(c_loc(transp), mu_ptr, c_loc(sig_tmp(1, 1)), d, &
    n, c_loc(x(1, 1)), ldX, c_null_ptr, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "mvn")
end procedure

module procedure int32_vec
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "int")
  c_ok = crp_int(x, int(size(x), c_size_t), m, n, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "int")
end procedure

module procedure int32_mat
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "int")
  c_ok = crp_int(x, int(size(x), c_size_t), m, n, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "int")
end procedure

module procedure int64_vec
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "int")
  c_ok = crp_long_long(x, int(size(x), c_size_t), m, n, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "int")
end procedure

module procedure int64_mat
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "int")
  c_ok = crp_long_long(x, int(size(x), c_size_t), m, n, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "int")
end procedure

module procedure int64_vec32
  integer(c_int64_t) :: m64, n64
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "int")
  m64 = int(m, c_int64_t)
  n64 = int(n, c_int64_t)
  c_ok = crp_long_long(x, int(size(x), c_size_t), m64, n64, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "int")
end procedure

module procedure int64_mat32
  integer(c_int64_t) :: m64, n64
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "int")
  m64 = int(m, c_int64_t)
  n64 = int(n, c_int64_t)
  c_ok = crp_long_long(x, int(size(x), c_size_t), m64, n64, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "int")
end procedure

module procedure perm32_vec
  integer(c_int64_t) :: n64, max_n
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "perm")
  n64 = int(size(x), c_int64_t)
  max_n = int(huge(0_c_int), c_int64_t) - 1_c_int64_t
  if (n64 > max_n) error stop "perm: size(x) too large"
  c_ok = crp_perm(x, int(n64, c_int), self%p)
  if (.not. c_ok) error stop rp_error_message(self, "perm")
  x = x + 1_c_int32_t
end procedure

module procedure perm64_vec
  integer(c_int32_t), allocatable :: tmp(:)
  integer(c_int64_t) :: n64, max_n
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "perm")
  n64 = int(size(x), c_int64_t)
  max_n = int(huge(0_c_int), c_int64_t) - 1_c_int64_t
  if (n64 > max_n) error stop "perm: size(x) too large"
  allocate(tmp(size(x)))
  c_ok = crp_perm(tmp, int(n64, c_int), self%p)
  if (.not. c_ok) error stop rp_error_message(self, "perm")
  x = int(tmp, c_int64_t) + 1_c_int64_t
end procedure

module procedure sample32_vec
  integer(c_int64_t) :: n64, k64, max_n
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "sample")
  n64 = int(n, c_int64_t)
  k64 = int(size(x), c_int64_t)
  max_n = int(huge(0_c_int), c_int64_t)
  if (n64 > max_n) error stop "sample: n too large"
  if (k64 > max_n) error stop "sample: size(x) too large"
  c_ok = crp_sample(x, int(n64, c_int), int(k64, c_int), self%p)
  if (.not. c_ok) error stop rp_error_message(self, "sample")
  x = x + 1_c_int32_t
end procedure

module procedure sample32_vec64
  integer(c_int64_t) :: k64, max_n
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "sample")
  k64 = int(size(x), c_int64_t)
  max_n = int(huge(0_c_int), c_int64_t)
  if (n > max_n) error stop "sample: n too large"
  if (k64 > max_n) error stop "sample: size(x) too large"
  c_ok = crp_sample(x, int(n, c_int), int(k64, c_int), self%p)
  if (.not. c_ok) error stop rp_error_message(self, "sample")
  x = x + 1_c_int32_t
end procedure

module procedure sample64_vec
  integer(c_int32_t), allocatable :: tmp(:)
  integer(c_int64_t) :: k64, max_n
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "sample")
  k64 = int(size(x), c_int64_t)
  max_n = int(huge(0_c_int), c_int64_t)
  if (n > max_n) error stop "sample: n too large"
  if (k64 > max_n) error stop "sample: size(x) too large"
  allocate(tmp(size(x)))
  c_ok = crp_sample(tmp, int(n, c_int), int(k64, c_int), self%p)
  if (.not. c_ok) error stop rp_error_message(self, "sample")
  x = int(tmp, c_int64_t) + 1_c_int64_t
end procedure

module procedure sample64_vec32
  integer(c_int32_t), allocatable :: tmp(:)
  integer(c_int64_t) :: n64, k64, max_n
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "sample")
  n64 = int(n, c_int64_t)
  k64 = int(size(x), c_int64_t)
  max_n = int(huge(0_c_int), c_int64_t)
  if (n64 > max_n) error stop "sample: n too large"
  if (k64 > max_n) error stop "sample: size(x) too large"
  allocate(tmp(size(x)))
  c_ok = crp_sample(tmp, int(n64, c_int), int(k64, c_int), self%p)
  if (.not. c_ok) error stop rp_error_message(self, "sample")
  x = int(tmp, c_int64_t) + 1_c_int64_t
end procedure

module procedure raw32_vec
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "raw")
  c_ok = crp_uint32(x, int(size(x), c_size_t), 0_c_int32_t, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "raw")
end procedure

module procedure raw32_mat
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "raw")
  c_ok = crp_uint32(x, int(size(x), c_size_t), 0_c_int32_t, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "raw")
end procedure

module procedure raw64_vec
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "raw")
  c_ok = crp_uint64(x, int(size(x), c_size_t), 0_c_int64_t, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "raw")
end procedure

module procedure raw64_mat
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "raw")
  c_ok = crp_uint64(x, int(size(x), c_size_t), 0_c_int64_t, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "raw")
end procedure

module procedure rp_serialize
  integer(c_int) :: len
  integer(c_int8_t), allocatable, target :: tmp(:)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "serialize")
  len = 0_c_int
  c_ok = crp_serialize(c_null_ptr, len, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "serialize")
  if (len <= 0_c_int) then
    allocate(buf(0))
    return
  end if
  allocate(tmp(int(len)))
  c_ok = crp_serialize(c_loc(tmp(1)), len, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "serialize")
  allocate(buf(int(len)))
  buf = tmp
end procedure

module procedure rp_deserialize
  integer(c_int8_t), allocatable, target :: tmp(:)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "deserialize")
  if (size(buf) == 0) then
    error stop "deserialize: empty buffer"
  end if
  allocate(tmp(size(buf)))
  tmp = buf
  c_ok = crp_deserialize(c_loc(tmp(1)), int(size(tmp), c_int), self%p)
  if (.not. c_ok) error stop rp_error_message(self, "deserialize")
end procedure

module procedure set_state32
  logical(c_bool) :: c_ok
  integer(c_int64_t), allocatable, target :: tmp(:)
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "set_state")
  if (size(state) <= 0) then
    error stop "set_state: empty state"
  end if
  allocate(tmp(size(state)))
  tmp = int(state, c_int64_t)
  c_ok = crp_set_state(tmp, int(size(tmp), c_int), self%p)
  if (.not. c_ok) error stop rp_error_message(self, "set_state")
end procedure

module procedure set_state64
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "set_state")
  if (size(state) <= 0) then
    error stop "set_state: empty state"
  end if
  c_ok = crp_set_state(state, int(size(state), c_int), self%p)
  if (.not. c_ok) error stop rp_error_message(self, "set_state")
end procedure

module procedure rp_advance32
  integer(c_int64_t) :: delta64(2)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "advance")
  if (delta < 0_c_int32_t) error stop "advance: delta must be nonnegative"
  delta64 = [int(delta, c_int64_t), 0_c_int64_t]
  c_ok = crp_advance(delta64, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "advance")
end procedure

module procedure rp_advance64
  integer(c_int64_t) :: delta64(2)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "advance")
  if (delta < 0_c_int64_t) error stop "advance: delta must be nonnegative"
  delta64 = [delta, 0_c_int64_t]
  c_ok = crp_advance(delta64, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "advance")
end procedure

module procedure rp_advance128
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "advance")
  c_ok = crp_advance(delta, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "advance")
end procedure

module procedure rp_philox_set_key
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "philox_set_key")
  c_ok = crp_philox_set_key(key, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "philox_set_key")
end procedure

module procedure rp_pcg64_set_inc
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "pcg64_set_inc")
  c_ok = crp_pcg64_set_inc(inc, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "pcg64_set_inc")
end procedure

module procedure rp_sfc64_set_abc
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "sfc64_set_abc")
  c_ok = crp_sfc64_set_abc(abc, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "sfc64_set_abc")
end procedure

module procedure rp_chacha_set_nonce
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "chacha_set_nonce")
  c_ok = crp_chacha_set_nonce(nonce, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "chacha_set_nonce")
end procedure

module procedure rp_squares_set_key32
  integer(c_int64_t) :: ckey
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "squares_set_key")
  ckey = int(key, c_int64_t)
  c_ok = crp_squares_set_key(ckey, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "squares_set_key")
end procedure

module procedure rp_squares_set_key64
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "squares_set_key")
  c_ok = crp_squares_set_key(key, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "squares_set_key")
end procedure

module procedure rp_last_error
  type(c_ptr) :: p
  if (.not. c_associated(self%p)) then
    msg = ""
    return
  end if
  p = crp_last_error(self%p)
  msg = cstr_to_fstring(p)
end procedure
end submodule randompack_impl
