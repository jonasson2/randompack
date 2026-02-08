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

subroutine rp_create(self, engine, ok)
  class(randompack_rng), intent(inout) :: self
  character(len=*), intent(in), optional :: engine
  character(kind=c_char), allocatable, target :: eng(:)
  logical, intent(out), optional :: ok
  call check_kinds()
  if (c_associated(self%p)) call self%free()
  if (present(engine)) then
    eng = to_c_string(engine)
    self%p = crp_create(c_loc(eng(1)))
  else
    self%p = crp_create(c_null_ptr)
  end if
  if (.not. c_associated(self%p)) then
    if (present(ok)) then
      ok = .false.
      return
    end if
    call rp_die(self, "create")
  end if
  if (present(ok)) ok = .true.
end subroutine

subroutine rp_duplicate(self, out, ok)
  class(randompack_rng), intent(in) :: self
  type(randompack_rng), intent(out) :: out
  logical, intent(out), optional :: ok
  if (.not. rp_check(self, "duplicate", ok)) return
  out%p = crp_duplicate(self%p)
  if (.not. c_associated(out%p)) then
    if (present(ok)) then
      ok = .false.
      return
    end if
    call rp_die(self, "duplicate")
  end if
  if (present(ok)) ok = .true.
end subroutine

subroutine rp_randomize(self, ok)
  class(randompack_rng), intent(inout) :: self
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "randomize", ok)) return
  c_ok = crp_randomize(self%p)
  if (.not. rp_check_ok(self, "randomize", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine rp_full_mantissa(self, enable, ok)
  class(randompack_rng), intent(inout) :: self
  logical, intent(in) :: enable
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "full_mantissa", ok)) return
  c_ok = crp_full_mantissa(self%p, merge(.true._c_bool, .false._c_bool, enable))
  if (.not. rp_check_ok(self, "full_mantissa", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine rp_seed(self, seed, spawn_key, ok)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(in) :: seed
  integer(c_int32_t), intent(in), optional, target :: spawn_key(:)
  type(c_ptr) :: keyp
  integer(c_int) :: nkey
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "seed", ok)) return
  if (present(spawn_key)) then
    keyp = c_loc(spawn_key(1))
    nkey = int(size(spawn_key), c_int)
  else
    keyp = c_null_ptr
    nkey = 0_c_int
  end if
  c_ok = crp_seed(seed, keyp, nkey, self%p)
  if (.not. rp_check_ok(self, "seed", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine u01_vec(self, x, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "u01", ok)) return
  c_ok = crp_u01(x, int(size(x), c_size_t), self%p)
  if (.not. rp_check_ok(self, "u01", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine u01_mat(self, x, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "u01", ok)) return
  c_ok = crp_u01(x, int(size(x), c_size_t), self%p)
  if (.not. rp_check_ok(self, "u01", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine u01f_vec(self, x, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "u01", ok)) return
  c_ok = crp_u01f(x, int(size(x), c_size_t), self%p)
  if (.not. rp_check_ok(self, "u01", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine u01f_mat(self, x, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "u01", ok)) return
  c_ok = crp_u01f(x, int(size(x), c_size_t), self%p)
  if (.not. rp_check_ok(self, "u01", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine unif_vec(self, x, a, b, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: a
  double precision, intent(in) :: b
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "unif", ok)) return
  c_ok = crp_unif(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. rp_check_ok(self, "unif", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine unif_mat(self, x, a, b, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: a
  double precision, intent(in) :: b
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "unif", ok)) return
  c_ok = crp_unif(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. rp_check_ok(self, "unif", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine uniff_vec(self, x, a, b, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: a
  real, intent(in) :: b
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "unif", ok)) return
  c_ok = crp_uniff(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. rp_check_ok(self, "unif", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine uniff_mat(self, x, a, b, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: a
  real, intent(in) :: b
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "unif", ok)) return
  c_ok = crp_uniff(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. rp_check_ok(self, "unif", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine norm_vec(self, x, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "norm", ok)) return
  c_ok = crp_norm(x, int(size(x), c_size_t), self%p)
  if (.not. rp_check_ok(self, "norm", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine norm_mat(self, x, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "norm", ok)) return
  c_ok = crp_norm(x, int(size(x), c_size_t), self%p)
  if (.not. rp_check_ok(self, "norm", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine normf_vec(self, x, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "norm", ok)) return
  c_ok = crp_normf(x, int(size(x), c_size_t), self%p)
  if (.not. rp_check_ok(self, "norm", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine normf_mat(self, x, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "norm", ok)) return
  c_ok = crp_normf(x, int(size(x), c_size_t), self%p)
  if (.not. rp_check_ok(self, "norm", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine normal_vec(self, x, mu, sigma, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: mu
  double precision, intent(in) :: sigma
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "normal", ok)) return
  c_ok = crp_normal(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. rp_check_ok(self, "normal", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine normal_mat(self, x, mu, sigma, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: mu
  double precision, intent(in) :: sigma
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "normal", ok)) return
  c_ok = crp_normal(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. rp_check_ok(self, "normal", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine normalf_vec(self, x, mu, sigma, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: mu
  real, intent(in) :: sigma
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "normal", ok)) return
  c_ok = crp_normalf(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. rp_check_ok(self, "normal", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine normalf_mat(self, x, mu, sigma, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: mu
  real, intent(in) :: sigma
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "normal", ok)) return
  c_ok = crp_normalf(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. rp_check_ok(self, "normal", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine exp_vec(self, x, scale, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: scale
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "exp", ok)) return
  c_ok = crp_exp(x, int(size(x), c_size_t), scale, self%p)
  if (.not. rp_check_ok(self, "exp", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine exp_mat(self, x, scale, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: scale
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "exp", ok)) return
  c_ok = crp_exp(x, int(size(x), c_size_t), scale, self%p)
  if (.not. rp_check_ok(self, "exp", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine expf_vec(self, x, scale, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: scale
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "exp", ok)) return
  c_ok = crp_expf(x, int(size(x), c_size_t), scale, self%p)
  if (.not. rp_check_ok(self, "exp", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine expf_mat(self, x, scale, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: scale
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "exp", ok)) return
  c_ok = crp_expf(x, int(size(x), c_size_t), scale, self%p)
  if (.not. rp_check_ok(self, "exp", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine lognormal_vec(self, x, mu, sigma, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: mu
  double precision, intent(in) :: sigma
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "lognormal", ok)) return
  c_ok = crp_lognormal(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. rp_check_ok(self, "lognormal", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine lognormal_mat(self, x, mu, sigma, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: mu
  double precision, intent(in) :: sigma
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "lognormal", ok)) return
  c_ok = crp_lognormal(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. rp_check_ok(self, "lognormal", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine lognormalf_vec(self, x, mu, sigma, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: mu
  real, intent(in) :: sigma
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "lognormal", ok)) return
  c_ok = crp_lognormalf(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. rp_check_ok(self, "lognormal", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine lognormalf_mat(self, x, mu, sigma, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: mu
  real, intent(in) :: sigma
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "lognormal", ok)) return
  c_ok = crp_lognormalf(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. rp_check_ok(self, "lognormal", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine gamma_vec(self, x, shape, scale, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: shape
  double precision, intent(in) :: scale
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "gamma", ok)) return
  c_ok = crp_gamma(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. rp_check_ok(self, "gamma", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine gamma_mat(self, x, shape, scale, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: shape
  double precision, intent(in) :: scale
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "gamma", ok)) return
  c_ok = crp_gamma(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. rp_check_ok(self, "gamma", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine gammaf_vec(self, x, shape, scale, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: shape
  real, intent(in) :: scale
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "gamma", ok)) return
  c_ok = crp_gammaf(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. rp_check_ok(self, "gamma", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine gammaf_mat(self, x, shape, scale, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: shape
  real, intent(in) :: scale
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "gamma", ok)) return
  c_ok = crp_gammaf(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. rp_check_ok(self, "gamma", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine beta_vec(self, x, a, b, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: a
  double precision, intent(in) :: b
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "beta", ok)) return
  c_ok = crp_beta(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. rp_check_ok(self, "beta", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine beta_mat(self, x, a, b, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: a
  double precision, intent(in) :: b
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "beta", ok)) return
  c_ok = crp_beta(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. rp_check_ok(self, "beta", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine betaf_vec(self, x, a, b, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: a
  real, intent(in) :: b
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "beta", ok)) return
  c_ok = crp_betaf(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. rp_check_ok(self, "beta", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine betaf_mat(self, x, a, b, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: a
  real, intent(in) :: b
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "beta", ok)) return
  c_ok = crp_betaf(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. rp_check_ok(self, "beta", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine chi2_vec(self, x, nu, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: nu
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "chi2", ok)) return
  c_ok = crp_chi2(x, int(size(x), c_size_t), nu, self%p)
  if (.not. rp_check_ok(self, "chi2", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine chi2_mat(self, x, nu, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: nu
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "chi2", ok)) return
  c_ok = crp_chi2(x, int(size(x), c_size_t), nu, self%p)
  if (.not. rp_check_ok(self, "chi2", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine chi2f_vec(self, x, nu, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: nu
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "chi2", ok)) return
  c_ok = crp_chi2f(x, int(size(x), c_size_t), nu, self%p)
  if (.not. rp_check_ok(self, "chi2", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine chi2f_mat(self, x, nu, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: nu
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "chi2", ok)) return
  c_ok = crp_chi2f(x, int(size(x), c_size_t), nu, self%p)
  if (.not. rp_check_ok(self, "chi2", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine t_vec(self, x, nu, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: nu
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "t", ok)) return
  c_ok = crp_t(x, int(size(x), c_size_t), nu, self%p)
  if (.not. rp_check_ok(self, "t", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine t_mat(self, x, nu, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: nu
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "t", ok)) return
  c_ok = crp_t(x, int(size(x), c_size_t), nu, self%p)
  if (.not. rp_check_ok(self, "t", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine tf_vec(self, x, nu, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: nu
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "t", ok)) return
  c_ok = crp_tf(x, int(size(x), c_size_t), nu, self%p)
  if (.not. rp_check_ok(self, "t", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine tf_mat(self, x, nu, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: nu
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "t", ok)) return
  c_ok = crp_tf(x, int(size(x), c_size_t), nu, self%p)
  if (.not. rp_check_ok(self, "t", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine f_vec(self, x, nu1, nu2, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: nu1
  double precision, intent(in) :: nu2
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "f", ok)) return
  c_ok = crp_f(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. rp_check_ok(self, "f", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine f_mat(self, x, nu1, nu2, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: nu1
  double precision, intent(in) :: nu2
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "f", ok)) return
  c_ok = crp_f(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. rp_check_ok(self, "f", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine ff_vec(self, x, nu1, nu2, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: nu1
  real, intent(in) :: nu2
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "f", ok)) return
  c_ok = crp_ff(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. rp_check_ok(self, "f", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine ff_mat(self, x, nu1, nu2, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: nu1
  real, intent(in) :: nu2
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "f", ok)) return
  c_ok = crp_ff(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. rp_check_ok(self, "f", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine gumbel_vec(self, x, mu, beta, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: mu
  double precision, intent(in) :: beta
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "gumbel", ok)) return
  c_ok = crp_gumbel(x, int(size(x), c_size_t), mu, beta, self%p)
  if (.not. rp_check_ok(self, "gumbel", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine gumbel_mat(self, x, mu, beta, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: mu
  double precision, intent(in) :: beta
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "gumbel", ok)) return
  c_ok = crp_gumbel(x, int(size(x), c_size_t), mu, beta, self%p)
  if (.not. rp_check_ok(self, "gumbel", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine gumbelf_vec(self, x, mu, beta, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: mu
  real, intent(in) :: beta
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "gumbel", ok)) return
  c_ok = crp_gumbelf(x, int(size(x), c_size_t), mu, beta, self%p)
  if (.not. rp_check_ok(self, "gumbel", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine gumbelf_mat(self, x, mu, beta, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: mu
  real, intent(in) :: beta
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "gumbel", ok)) return
  c_ok = crp_gumbelf(x, int(size(x), c_size_t), mu, beta, self%p)
  if (.not. rp_check_ok(self, "gumbel", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine pareto_vec(self, x, xm, alpha, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: xm
  double precision, intent(in) :: alpha
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "pareto", ok)) return
  c_ok = crp_pareto(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. rp_check_ok(self, "pareto", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine pareto_mat(self, x, xm, alpha, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: xm
  double precision, intent(in) :: alpha
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "pareto", ok)) return
  c_ok = crp_pareto(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. rp_check_ok(self, "pareto", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine paretof_vec(self, x, xm, alpha, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: xm
  real, intent(in) :: alpha
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "pareto", ok)) return
  c_ok = crp_paretof(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. rp_check_ok(self, "pareto", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine paretof_mat(self, x, xm, alpha, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: xm
  real, intent(in) :: alpha
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "pareto", ok)) return
  c_ok = crp_paretof(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. rp_check_ok(self, "pareto", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine weibull_vec(self, x, shape, scale, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: shape
  double precision, intent(in) :: scale
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "weibull", ok)) return
  c_ok = crp_weibull(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. rp_check_ok(self, "weibull", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine weibull_mat(self, x, shape, scale, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: shape
  double precision, intent(in) :: scale
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "weibull", ok)) return
  c_ok = crp_weibull(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. rp_check_ok(self, "weibull", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine weibullf_vec(self, x, shape, scale, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: shape
  real, intent(in) :: scale
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "weibull", ok)) return
  c_ok = crp_weibullf(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. rp_check_ok(self, "weibull", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine weibullf_mat(self, x, shape, scale, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: shape
  real, intent(in) :: scale
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "weibull", ok)) return
  c_ok = crp_weibullf(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. rp_check_ok(self, "weibull", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine skew_normal_vec(self, x, mu, sigma, alpha, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: mu
  double precision, intent(in) :: sigma
  double precision, intent(in) :: alpha
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "skew_normal", ok)) return
  c_ok = crp_skew_normal(x, int(size(x), c_size_t), mu, sigma, alpha, self%p)
  if (.not. rp_check_ok(self, "skew_normal", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine skew_normal_mat(self, x, mu, sigma, alpha, ok)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: mu
  double precision, intent(in) :: sigma
  double precision, intent(in) :: alpha
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "skew_normal", ok)) return
  c_ok = crp_skew_normal(x, int(size(x), c_size_t), mu, sigma, alpha, self%p)
  if (.not. rp_check_ok(self, "skew_normal", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine skew_normalf_vec(self, x, mu, sigma, alpha, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: mu
  real, intent(in) :: sigma
  real, intent(in) :: alpha
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "skew_normal", ok)) return
  c_ok = crp_skew_normalf(x, int(size(x), c_size_t), mu, sigma, alpha, self%p)
  if (.not. rp_check_ok(self, "skew_normal", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine skew_normalf_mat(self, x, mu, sigma, alpha, ok)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: mu
  real, intent(in) :: sigma
  real, intent(in) :: alpha
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "skew_normal", ok)) return
  c_ok = crp_skew_normalf(x, int(size(x), c_size_t), mu, sigma, alpha, self%p)
  if (.not. rp_check_ok(self, "skew_normal", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine int32_vec(self, x, m, n, ok)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(out) :: x(:)
  integer(c_int32_t), intent(in) :: m
  integer(c_int32_t), intent(in) :: n
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "int", ok)) return
  c_ok = crp_int(x, int(size(x), c_size_t), m, n, self%p)
  if (.not. rp_check_ok(self, "int", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine int32_mat(self, x, m, n, ok)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(out) :: x(:,:)
  integer(c_int32_t), intent(in) :: m
  integer(c_int32_t), intent(in) :: n
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "int", ok)) return
  c_ok = crp_int(x, int(size(x), c_size_t), m, n, self%p)
  if (.not. rp_check_ok(self, "int", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine int64_vec(self, x, m, n, ok)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:)
  integer(c_int64_t), intent(in) :: m
  integer(c_int64_t), intent(in) :: n
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "int", ok)) return
  c_ok = crp_long_long(x, int(size(x), c_size_t), m, n, self%p)
  if (.not. rp_check_ok(self, "int", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine int64_mat(self, x, m, n, ok)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:,:)
  integer(c_int64_t), intent(in) :: m
  integer(c_int64_t), intent(in) :: n
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "int", ok)) return
  c_ok = crp_long_long(x, int(size(x), c_size_t), m, n, self%p)
  if (.not. rp_check_ok(self, "int", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine rp_serialize(self, buf, ok)
  class(randompack_rng), intent(inout) :: self
  integer(c_int8_t), allocatable, intent(out) :: buf(:)
  integer(c_int) :: len
  integer(c_int8_t), allocatable, target :: tmp(:)
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "serialize", ok)) return
  len = 0_c_int
  c_ok = crp_serialize(c_null_ptr, len, self%p)
  if (.not. rp_check_ok(self, "serialize", ok, c_ok)) return
  if (len <= 0_c_int) then
    allocate(buf(0))
    if (present(ok)) ok = .true.
    return
  end if
  allocate(tmp(int(len)))
  c_ok = crp_serialize(c_loc(tmp(1)), len, self%p)
  if (.not. rp_check_ok(self, "serialize", ok, c_ok)) return
  allocate(buf(int(len)))
  buf = tmp
  if (present(ok)) ok = .true.
end subroutine

subroutine rp_deserialize(self, buf, ok)
  class(randompack_rng), intent(inout) :: self
  integer(c_int8_t), intent(in) :: buf(:)
  integer(c_int8_t), allocatable, target :: tmp(:)
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "deserialize", ok)) return
  if (size(buf) == 0) then
    if (present(ok)) then
      ok = .false.
      return
    end if
    call rp_die(self, "deserialize")
  end if
  allocate(tmp(size(buf)))
  tmp = buf
  c_ok = crp_deserialize(c_loc(tmp(1)), int(size(tmp), c_int), self%p)
  if (.not. rp_check_ok(self, "deserialize", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine set_state32(self, state, ok)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(in) :: state(:)
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  integer(c_int64_t), allocatable, target :: tmp(:)
  if (.not. rp_check(self, "set_state", ok)) return
  if (size(state) <= 0) then
    if (present(ok)) then
      ok = .false.
      return
    end if
    call rp_die(self, "set_state")
  end if
  allocate(tmp(size(state)))
  tmp = int(state, c_int64_t)
  c_ok = crp_set_state(tmp, int(size(tmp), c_int), self%p)
  if (.not. rp_check_ok(self, "set_state", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine set_state64(self, state, ok)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in), target :: state(:)
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "set_state", ok)) return
  if (size(state) <= 0) then
    if (present(ok)) then
      ok = .false.
      return
    end if
    call rp_die(self, "set_state")
  end if
  c_ok = crp_set_state(state, int(size(state), c_int), self%p)
  if (.not. rp_check_ok(self, "set_state", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine
subroutine rp_philox_set_state(self, ctr, key, ok)
  class(randompack_rng), intent(inout) :: self
  type(randompack_philox_ctr), intent(in) :: ctr
  type(randompack_philox_key), intent(in) :: key
  type(c_philox_ctr) :: cctr
  type(c_philox_key) :: ckey
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "philox_set_state", ok)) return
  cctr%v = int(ctr%v, c_int64_t)
  ckey%v = int(key%v, c_int64_t)
  c_ok = crp_philox_set_state(cctr, ckey, self%p)
  if (.not. rp_check_ok(self, "philox_set_state", ok, c_ok)) return
  if (present(ok)) ok = .true.
end subroutine

subroutine rp_squares_set_state(self, ctr, key, ok)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: ctr
  integer(c_int64_t), intent(in) :: key
  logical, intent(out), optional :: ok
  logical(c_bool) :: c_ok
  if (.not. rp_check(self, "squares_set_state", ok)) return
  c_ok = crp_squares_set_state(ctr, key, self%p)
  if (.not. rp_check_ok(self, "squares_set_state", ok, c_ok)) return
  if (present(ok)) ok = .true.
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
