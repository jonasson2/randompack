!-*- fortran -*-
subroutine rp_finalize(self)
  type(randompack_rng), intent(inout) :: self
  call self%free()
end subroutine

subroutine rp_free(self)
  class(randompack_rng), intent(inout) :: self
  if (c_associated(self%p)) then
    call crp_free(self%p)
    self%p = c_null_ptr
  end if
end subroutine

subroutine rp_create(self, engine, bitexact)
  class(randompack_rng), intent(inout) :: self
  character(len=*), intent(in), optional :: engine
  logical, intent(in), optional :: bitexact
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
end subroutine

subroutine rp_duplicate(self, out)
  class(randompack_rng), intent(in) :: self
  type(randompack_rng), intent(out) :: out
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "duplicate")
  out%p = crp_duplicate(self%p)
  if (.not. c_associated(out%p)) error stop rp_error_message(self, "duplicate")
end subroutine

subroutine rp_randomize(self)
  class(randompack_rng), intent(inout) :: self
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "randomize")
  c_ok = crp_randomize(self%p)
  if (.not. c_ok) error stop rp_error_message(self, "randomize")
end subroutine

subroutine rp_full_mantissa(self, enable)
  class(randompack_rng), intent(inout) :: self
  logical, intent(in) :: enable
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "full_mantissa")
  c_ok = crp_full_mantissa(self%p, merge(.true._c_bool, .false._c_bool, enable))
  if (.not. c_ok) error stop rp_error_message(self, "full_mantissa")
end subroutine

subroutine rp_jump(self, p)
  class(randompack_rng), intent(inout) :: self
  integer(c_int), intent(in) :: p
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "jump")
  c_ok = crp_jump(p, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "jump")
end subroutine


subroutine rp_seed32(self, seed, spawn_key)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(in) :: seed
  integer(c_int32_t), intent(in), optional, target :: spawn_key(:)
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
end subroutine

subroutine rp_seed64(self, seed, spawn_key)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: seed
  integer(c_int64_t), intent(in), optional, target :: spawn_key(:)
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
end subroutine

subroutine unif_vec(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in), optional :: a
  double precision, intent(in), optional :: b
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
end subroutine

subroutine unif_mat(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in), optional :: a
  double precision, intent(in), optional :: b
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
end subroutine

subroutine uniff_vec(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in), optional :: a
  real, intent(in), optional :: b
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
end subroutine

subroutine uniff_mat(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in), optional :: a
  real, intent(in), optional :: b
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
end subroutine

subroutine normal_vec(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: sigma
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
end subroutine

subroutine normal_mat(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: sigma
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
end subroutine

subroutine normalf_vec(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: sigma
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
end subroutine

subroutine normalf_mat(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: sigma
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
end subroutine

subroutine exp_vec(self, x, scale)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in), optional :: scale
  double precision :: s0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "exp")
  s0 = 1
  if (present(scale)) s0 = scale
  if (.not. (s0 > 0)) error stop "exp: scale must be positive"
  c_ok = crp_exp(x, int(size(x), c_size_t), s0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "exp")
end subroutine

subroutine exp_mat(self, x, scale)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in), optional :: scale
  double precision :: s0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "exp")
  s0 = 1
  if (present(scale)) s0 = scale
  if (.not. (s0 > 0)) error stop "exp: scale must be positive"
  c_ok = crp_exp(x, int(size(x), c_size_t), s0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "exp")
end subroutine

subroutine expf_vec(self, x, scale)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in), optional :: scale
  real :: s0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "exp")
  s0 = 1
  if (present(scale)) s0 = scale
  if (.not. (s0 > 0)) error stop "exp: scale must be positive"
  c_ok = crp_expf(x, int(size(x), c_size_t), s0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "exp")
end subroutine

subroutine expf_mat(self, x, scale)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in), optional :: scale
  real :: s0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "exp")
  s0 = 1
  if (present(scale)) s0 = scale
  if (.not. (s0 > 0)) error stop "exp: scale must be positive"
  c_ok = crp_expf(x, int(size(x), c_size_t), s0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "exp")
end subroutine

subroutine lognormal_vec(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: sigma
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
end subroutine

subroutine lognormal_mat(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: sigma
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
end subroutine

subroutine lognormalf_vec(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: sigma
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
end subroutine

subroutine lognormalf_mat(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: sigma
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
end subroutine

subroutine gamma_vec(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: shape
  double precision, intent(in), optional :: scale
  double precision :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "gamma")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "gamma: shape/scale must be positive"
  c_ok = crp_gamma(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "gamma")
end subroutine

subroutine gamma_mat(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: shape
  double precision, intent(in), optional :: scale
  double precision :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "gamma")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "gamma: shape/scale must be positive"
  c_ok = crp_gamma(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "gamma")
end subroutine

subroutine gammaf_vec(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: shape
  real, intent(in), optional :: scale
  real :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "gamma")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "gamma: shape/scale must be positive"
  c_ok = crp_gammaf(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "gamma")
end subroutine

subroutine gammaf_mat(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: shape
  real, intent(in), optional :: scale
  real :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "gamma")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "gamma: shape/scale must be positive"
  c_ok = crp_gammaf(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "gamma")
end subroutine

subroutine beta_vec(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: a
  double precision, intent(in) :: b
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "beta")
  c_ok = crp_beta(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "beta")
end subroutine

subroutine beta_mat(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: a
  double precision, intent(in) :: b
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "beta")
  c_ok = crp_beta(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "beta")
end subroutine

subroutine betaf_vec(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: a
  real, intent(in) :: b
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "beta")
  c_ok = crp_betaf(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "beta")
end subroutine

subroutine betaf_mat(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: a
  real, intent(in) :: b
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "beta")
  c_ok = crp_betaf(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "beta")
end subroutine

subroutine chi2_vec(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: nu
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "chi2")
  c_ok = crp_chi2(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "chi2")
end subroutine

subroutine chi2_mat(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: nu
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "chi2")
  c_ok = crp_chi2(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "chi2")
end subroutine

subroutine chi2f_vec(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: nu
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "chi2")
  c_ok = crp_chi2f(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "chi2")
end subroutine

subroutine chi2f_mat(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: nu
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "chi2")
  c_ok = crp_chi2f(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "chi2")
end subroutine

subroutine t_vec(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: nu
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "t")
  c_ok = crp_t(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "t")
end subroutine

subroutine t_mat(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: nu
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "t")
  c_ok = crp_t(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "t")
end subroutine

subroutine tf_vec(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: nu
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "t")
  c_ok = crp_tf(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "t")
end subroutine

subroutine tf_mat(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: nu
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "t")
  c_ok = crp_tf(x, int(size(x), c_size_t), nu, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "t")
end subroutine

subroutine f_vec(self, x, nu1, nu2)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: nu1
  double precision, intent(in) :: nu2
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "f")
  c_ok = crp_f(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "f")
end subroutine

subroutine f_mat(self, x, nu1, nu2)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: nu1
  double precision, intent(in) :: nu2
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "f")
  c_ok = crp_f(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "f")
end subroutine

subroutine ff_vec(self, x, nu1, nu2)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: nu1
  real, intent(in) :: nu2
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "f")
  c_ok = crp_ff(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "f")
end subroutine

subroutine ff_mat(self, x, nu1, nu2)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: nu1
  real, intent(in) :: nu2
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "f")
  c_ok = crp_ff(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "f")
end subroutine

subroutine gumbel_vec(self, x, mu, beta)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: beta
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
end subroutine

subroutine gumbel_mat(self, x, mu, beta)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: beta
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
end subroutine

subroutine gumbelf_vec(self, x, mu, beta)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: beta
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
end subroutine

subroutine gumbelf_mat(self, x, mu, beta)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: beta
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
end subroutine

subroutine pareto_vec(self, x, xm, alpha)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: xm
  double precision, intent(in) :: alpha
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "pareto")
  c_ok = crp_pareto(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "pareto")
end subroutine

subroutine pareto_mat(self, x, xm, alpha)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: xm
  double precision, intent(in) :: alpha
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "pareto")
  c_ok = crp_pareto(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "pareto")
end subroutine

subroutine paretof_vec(self, x, xm, alpha)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: xm
  real, intent(in) :: alpha
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "pareto")
  c_ok = crp_paretof(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "pareto")
end subroutine

subroutine paretof_mat(self, x, xm, alpha)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: xm
  real, intent(in) :: alpha
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "pareto")
  c_ok = crp_paretof(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "pareto")
end subroutine

subroutine weibull_vec(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: shape
  double precision, intent(in), optional :: scale
  double precision :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "weibull")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "weibull: shape/scale must be positive"
  c_ok = crp_weibull(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "weibull")
end subroutine

subroutine weibull_mat(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: shape
  double precision, intent(in), optional :: scale
  double precision :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "weibull")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "weibull: shape/scale must be positive"
  c_ok = crp_weibull(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "weibull")
end subroutine

subroutine weibullf_vec(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: shape
  real, intent(in), optional :: scale
  real :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "weibull")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "weibull: shape/scale must be positive"
  c_ok = crp_weibullf(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "weibull")
end subroutine

subroutine weibullf_mat(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: shape
  real, intent(in), optional :: scale
  real :: sc0
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "weibull")
  sc0 = 1
  if (present(scale)) sc0 = scale
  if (.not. (shape > 0 .and. sc0 > 0)) error stop "weibull: shape/scale must be positive"
  c_ok = crp_weibullf(x, int(size(x), c_size_t), shape, sc0, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "weibull")
end subroutine

subroutine skew_normal_vec(self, x, mu, sigma, alpha)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: sigma
  double precision, intent(in) :: alpha
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
end subroutine

subroutine skew_normal_mat(self, x, mu, sigma, alpha)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: sigma
  double precision, intent(in) :: alpha
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
end subroutine

subroutine skew_normalf_vec(self, x, mu, sigma, alpha)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: sigma
  real, intent(in) :: alpha
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
end subroutine

subroutine skew_normalf_mat(self, x, mu, sigma, alpha)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: sigma
  real, intent(in) :: alpha
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
end subroutine

subroutine int32_vec(self, x, m, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(out) :: x(:)
  integer(c_int32_t), intent(in) :: m
  integer(c_int32_t), intent(in) :: n
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "int")
  c_ok = crp_int(x, int(size(x), c_size_t), m, n, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "int")
end subroutine

subroutine int32_mat(self, x, m, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(out) :: x(:,:)
  integer(c_int32_t), intent(in) :: m
  integer(c_int32_t), intent(in) :: n
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "int")
  c_ok = crp_int(x, int(size(x), c_size_t), m, n, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "int")
end subroutine

subroutine int64_vec(self, x, m, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:)
  integer(c_int64_t), intent(in) :: m
  integer(c_int64_t), intent(in) :: n
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "int")
  c_ok = crp_long_long(x, int(size(x), c_size_t), m, n, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "int")
end subroutine

subroutine int64_mat(self, x, m, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:,:)
  integer(c_int64_t), intent(in) :: m
  integer(c_int64_t), intent(in) :: n
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "int")
  c_ok = crp_long_long(x, int(size(x), c_size_t), m, n, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "int")
end subroutine

subroutine int64_vec32(self, x, m, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:)
  integer(c_int32_t), intent(in) :: m
  integer(c_int32_t), intent(in) :: n
  integer(c_int64_t) :: m64, n64
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "int")
  m64 = int(m, c_int64_t)
  n64 = int(n, c_int64_t)
  c_ok = crp_long_long(x, int(size(x), c_size_t), m64, n64, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "int")
end subroutine

subroutine int64_mat32(self, x, m, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:,:)
  integer(c_int32_t), intent(in) :: m
  integer(c_int32_t), intent(in) :: n
  integer(c_int64_t) :: m64, n64
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "int")
  m64 = int(m, c_int64_t)
  n64 = int(n, c_int64_t)
  c_ok = crp_long_long(x, int(size(x), c_size_t), m64, n64, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "int")
end subroutine

subroutine raw32_vec(self, x)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(out) :: x(:)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "raw")
  c_ok = crp_uint32(x, int(size(x), c_size_t), 0_c_int32_t, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "raw")
end subroutine

subroutine raw32_mat(self, x)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(out) :: x(:,:)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "raw")
  c_ok = crp_uint32(x, int(size(x), c_size_t), 0_c_int32_t, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "raw")
end subroutine

subroutine raw64_vec(self, x)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "raw")
  c_ok = crp_uint64(x, int(size(x), c_size_t), 0_c_int64_t, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "raw")
end subroutine

subroutine raw64_mat(self, x)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:,:)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "raw")
  c_ok = crp_uint64(x, int(size(x), c_size_t), 0_c_int64_t, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "raw")
end subroutine

subroutine rp_serialize(self, buf)
  class(randompack_rng), intent(inout) :: self
  integer(c_int8_t), allocatable, intent(out) :: buf(:)
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
end subroutine

subroutine rp_deserialize(self, buf)
  class(randompack_rng), intent(inout) :: self
  integer(c_int8_t), intent(in) :: buf(:)
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
end subroutine

subroutine set_state32(self, state)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(in) :: state(:)
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
end subroutine

subroutine set_state64(self, state)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in), target :: state(:)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "set_state")
  if (size(state) <= 0) then
    error stop "set_state: empty state"
  end if
  c_ok = crp_set_state(state, int(size(state), c_int), self%p)
  if (.not. c_ok) error stop rp_error_message(self, "set_state")
end subroutine
subroutine rp_philox_set_ctr(self, ctr)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: ctr(4)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "philox_set_ctr")
  c_ok = crp_philox_set_ctr(ctr, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "philox_set_ctr")
end subroutine

subroutine rp_philox_set_key(self, key)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: key(2)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "philox_set_key")
  c_ok = crp_philox_set_key(key, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "philox_set_key")
end subroutine

subroutine rp_pcg64_set_inc(self, inc)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: inc(2)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "pcg64_set_inc")
  c_ok = crp_pcg64_set_inc(inc, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "pcg64_set_inc")
end subroutine

subroutine rp_sfc64_set_abc(self, abc)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: abc(3)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "sfc64_set_abc")
  c_ok = crp_sfc64_set_abc(abc, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "sfc64_set_abc")
end subroutine

subroutine rp_squares_set_ctr32(self, ctr)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(in) :: ctr
  integer(c_int64_t) :: cctr
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "squares_set_ctr")
  cctr = int(ctr, c_int64_t)
  c_ok = crp_squares_set_ctr(cctr, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "squares_set_ctr")
end subroutine

subroutine rp_squares_set_ctr64(self, ctr)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: ctr
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "squares_set_ctr")
  c_ok = crp_squares_set_ctr(ctr, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "squares_set_ctr")
end subroutine

subroutine rp_squares_set_key32(self, key)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(in) :: key
  integer(c_int64_t) :: ckey
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "squares_set_key")
  ckey = int(key, c_int64_t)
  c_ok = crp_squares_set_key(ckey, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "squares_set_key")
end subroutine

subroutine rp_squares_set_key64(self, key)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: key
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "squares_set_key")
  c_ok = crp_squares_set_key(key, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "squares_set_key")
end subroutine

function rp_last_error(self) result(msg)
  class(randompack_rng), intent(in) :: self
  character(len=:), allocatable :: msg
  type(c_ptr) :: p
  if (.not. c_associated(self%p)) then
    msg = ""
    return
  end if
  p = crp_last_error(self%p)
  msg = cstr_to_fstring(p)
end function
