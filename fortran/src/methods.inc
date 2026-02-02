subroutine rp_finalize(self)
  type(rng), intent(inout) :: self
  call self%free()
end subroutine

subroutine rp_free(self)
  class(rng), intent(inout) :: self
  if (c_associated(self%p)) then
    call crp_free(self%p)
    self%p = c_null_ptr
  end if
end subroutine

subroutine rp_create(self, engine)
  class(rng), intent(inout) :: self
  character(len=*), intent(in), optional :: engine
  character(kind=c_char), allocatable :: eng(:)
  if (c_associated(self%p)) call self%free()
  if (present(engine)) then
    eng = to_c_string(engine)
  else
    eng = [c_null_char]
  end if
  self%p = crp_create(eng)
  if (.not. c_associated(self%p)) call rp_die(self, "create")
end subroutine

function rp_duplicate(self) result(out)
  class(rng), intent(in) :: self
  type(rng) :: out
  if (.not. c_associated(self%p)) call rp_die(self, "duplicate")
  out%p = crp_duplicate(self%p)
  if (.not. c_associated(out%p)) call rp_die(self, "duplicate")
end function

subroutine rp_randomize(self)
  class(rng), intent(inout) :: self
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "randomize")
  ok = crp_randomize(self%p)
  if (.not. ok) call rp_die(self, "randomize")
end subroutine

subroutine rp_full_mantissa(self, enable)
  class(rng), intent(inout) :: self
  logical, intent(in) :: enable
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "full_mantissa")
  ok = crp_full_mantissa(self%p, merge(.true._c_bool, .false._c_bool, enable))
  if (.not. ok) call rp_die(self, "full_mantissa")
end subroutine

subroutine rp_seed(self, seed, spawn_key)
  class(rng), intent(inout) :: self
  integer, intent(in) :: seed
  integer, intent(in), optional, target :: spawn_key(:)
  type(c_ptr) :: keyp
  integer(c_int) :: nkey
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "seed")
  if (present(spawn_key)) then
    keyp = c_loc(spawn_key(1))
    nkey = int(size(spawn_key), c_int)
  else
    keyp = c_null_ptr
    nkey = 0_c_int
  end if
  ok = crp_seed(int(seed, c_int), keyp, nkey, self%p)
  if (.not. ok) call rp_die(self, "seed")
end subroutine

subroutine u01_vec(self, x)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:)
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "u01")
  ok = crp_u01(x, int(size(x), c_size_t), self%p)
  if (.not. ok) call rp_die(self, "u01")
end subroutine

subroutine u01_mat(self, x)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:,:)
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "u01")
  ok = crp_u01(x, int(size(x), c_size_t), self%p)
  if (.not. ok) call rp_die(self, "u01")
end subroutine

subroutine u01f_vec(self, x)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:)
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "u01")
  ok = crp_u01f(x, int(size(x), c_size_t), self%p)
  if (.not. ok) call rp_die(self, "u01")
end subroutine

subroutine u01f_mat(self, x)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:,:)
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "u01")
  ok = crp_u01f(x, int(size(x), c_size_t), self%p)
  if (.not. ok) call rp_die(self, "u01")
end subroutine

subroutine unif_vec(self, x, a, b)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:)
  real(c_double), intent(in) :: a
  real(c_double), intent(in) :: b
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "unif")
  ok = crp_unif(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. ok) call rp_die(self, "unif")
end subroutine

subroutine unif_mat(self, x, a, b)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:,:)
  real(c_double), intent(in) :: a
  real(c_double), intent(in) :: b
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "unif")
  ok = crp_unif(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. ok) call rp_die(self, "unif")
end subroutine

subroutine uniff_vec(self, x, a, b)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:)
  real(c_float), intent(in) :: a
  real(c_float), intent(in) :: b
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "unif")
  ok = crp_uniff(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. ok) call rp_die(self, "unif")
end subroutine

subroutine uniff_mat(self, x, a, b)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:,:)
  real(c_float), intent(in) :: a
  real(c_float), intent(in) :: b
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "unif")
  ok = crp_uniff(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. ok) call rp_die(self, "unif")
end subroutine

subroutine norm_vec(self, x)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:)
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "norm")
  ok = crp_norm(x, int(size(x), c_size_t), self%p)
  if (.not. ok) call rp_die(self, "norm")
end subroutine

subroutine norm_mat(self, x)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:,:)
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "norm")
  ok = crp_norm(x, int(size(x), c_size_t), self%p)
  if (.not. ok) call rp_die(self, "norm")
end subroutine

subroutine normf_vec(self, x)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:)
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "norm")
  ok = crp_normf(x, int(size(x), c_size_t), self%p)
  if (.not. ok) call rp_die(self, "norm")
end subroutine

subroutine normf_mat(self, x)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:,:)
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "norm")
  ok = crp_normf(x, int(size(x), c_size_t), self%p)
  if (.not. ok) call rp_die(self, "norm")
end subroutine

subroutine normal_vec(self, x, mu, sigma)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:)
  real(c_double), intent(in) :: mu
  real(c_double), intent(in) :: sigma
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "normal")
  ok = crp_normal(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. ok) call rp_die(self, "normal")
end subroutine

subroutine normal_mat(self, x, mu, sigma)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:,:)
  real(c_double), intent(in) :: mu
  real(c_double), intent(in) :: sigma
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "normal")
  ok = crp_normal(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. ok) call rp_die(self, "normal")
end subroutine

subroutine normalf_vec(self, x, mu, sigma)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:)
  real(c_float), intent(in) :: mu
  real(c_float), intent(in) :: sigma
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "normal")
  ok = crp_normalf(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. ok) call rp_die(self, "normal")
end subroutine

subroutine normalf_mat(self, x, mu, sigma)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:,:)
  real(c_float), intent(in) :: mu
  real(c_float), intent(in) :: sigma
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "normal")
  ok = crp_normalf(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. ok) call rp_die(self, "normal")
end subroutine

subroutine exp_vec(self, x, scale)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:)
  real(c_double), intent(in) :: scale
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "exp")
  ok = crp_exp(x, int(size(x), c_size_t), scale, self%p)
  if (.not. ok) call rp_die(self, "exp")
end subroutine

subroutine exp_mat(self, x, scale)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:,:)
  real(c_double), intent(in) :: scale
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "exp")
  ok = crp_exp(x, int(size(x), c_size_t), scale, self%p)
  if (.not. ok) call rp_die(self, "exp")
end subroutine

subroutine expf_vec(self, x, scale)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:)
  real(c_float), intent(in) :: scale
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "exp")
  ok = crp_expf(x, int(size(x), c_size_t), scale, self%p)
  if (.not. ok) call rp_die(self, "exp")
end subroutine

subroutine expf_mat(self, x, scale)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:,:)
  real(c_float), intent(in) :: scale
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "exp")
  ok = crp_expf(x, int(size(x), c_size_t), scale, self%p)
  if (.not. ok) call rp_die(self, "exp")
end subroutine

subroutine lognormal_vec(self, x, mu, sigma)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:)
  real(c_double), intent(in) :: mu
  real(c_double), intent(in) :: sigma
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "lognormal")
  ok = crp_lognormal(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. ok) call rp_die(self, "lognormal")
end subroutine

subroutine lognormal_mat(self, x, mu, sigma)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:,:)
  real(c_double), intent(in) :: mu
  real(c_double), intent(in) :: sigma
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "lognormal")
  ok = crp_lognormal(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. ok) call rp_die(self, "lognormal")
end subroutine

subroutine lognormalf_vec(self, x, mu, sigma)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:)
  real(c_float), intent(in) :: mu
  real(c_float), intent(in) :: sigma
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "lognormal")
  ok = crp_lognormalf(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. ok) call rp_die(self, "lognormal")
end subroutine

subroutine lognormalf_mat(self, x, mu, sigma)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:,:)
  real(c_float), intent(in) :: mu
  real(c_float), intent(in) :: sigma
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "lognormal")
  ok = crp_lognormalf(x, int(size(x), c_size_t), mu, sigma, self%p)
  if (.not. ok) call rp_die(self, "lognormal")
end subroutine

subroutine gamma_vec(self, x, shape, scale)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:)
  real(c_double), intent(in) :: shape
  real(c_double), intent(in) :: scale
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "gamma")
  ok = crp_gamma(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. ok) call rp_die(self, "gamma")
end subroutine

subroutine gamma_mat(self, x, shape, scale)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:,:)
  real(c_double), intent(in) :: shape
  real(c_double), intent(in) :: scale
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "gamma")
  ok = crp_gamma(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. ok) call rp_die(self, "gamma")
end subroutine

subroutine gammaf_vec(self, x, shape, scale)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:)
  real(c_float), intent(in) :: shape
  real(c_float), intent(in) :: scale
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "gamma")
  ok = crp_gammaf(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. ok) call rp_die(self, "gamma")
end subroutine

subroutine gammaf_mat(self, x, shape, scale)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:,:)
  real(c_float), intent(in) :: shape
  real(c_float), intent(in) :: scale
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "gamma")
  ok = crp_gammaf(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. ok) call rp_die(self, "gamma")
end subroutine

subroutine beta_vec(self, x, a, b)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:)
  real(c_double), intent(in) :: a
  real(c_double), intent(in) :: b
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "beta")
  ok = crp_beta(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. ok) call rp_die(self, "beta")
end subroutine

subroutine beta_mat(self, x, a, b)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:,:)
  real(c_double), intent(in) :: a
  real(c_double), intent(in) :: b
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "beta")
  ok = crp_beta(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. ok) call rp_die(self, "beta")
end subroutine

subroutine betaf_vec(self, x, a, b)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:)
  real(c_float), intent(in) :: a
  real(c_float), intent(in) :: b
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "beta")
  ok = crp_betaf(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. ok) call rp_die(self, "beta")
end subroutine

subroutine betaf_mat(self, x, a, b)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:,:)
  real(c_float), intent(in) :: a
  real(c_float), intent(in) :: b
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "beta")
  ok = crp_betaf(x, int(size(x), c_size_t), a, b, self%p)
  if (.not. ok) call rp_die(self, "beta")
end subroutine

subroutine chi2_vec(self, x, nu)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:)
  real(c_double), intent(in) :: nu
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "chi2")
  ok = crp_chi2(x, int(size(x), c_size_t), nu, self%p)
  if (.not. ok) call rp_die(self, "chi2")
end subroutine

subroutine chi2_mat(self, x, nu)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:,:)
  real(c_double), intent(in) :: nu
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "chi2")
  ok = crp_chi2(x, int(size(x), c_size_t), nu, self%p)
  if (.not. ok) call rp_die(self, "chi2")
end subroutine

subroutine chi2f_vec(self, x, nu)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:)
  real(c_float), intent(in) :: nu
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "chi2")
  ok = crp_chi2f(x, int(size(x), c_size_t), nu, self%p)
  if (.not. ok) call rp_die(self, "chi2")
end subroutine

subroutine chi2f_mat(self, x, nu)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:,:)
  real(c_float), intent(in) :: nu
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "chi2")
  ok = crp_chi2f(x, int(size(x), c_size_t), nu, self%p)
  if (.not. ok) call rp_die(self, "chi2")
end subroutine

subroutine t_vec(self, x, nu)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:)
  real(c_double), intent(in) :: nu
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "t")
  ok = crp_t(x, int(size(x), c_size_t), nu, self%p)
  if (.not. ok) call rp_die(self, "t")
end subroutine

subroutine t_mat(self, x, nu)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:,:)
  real(c_double), intent(in) :: nu
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "t")
  ok = crp_t(x, int(size(x), c_size_t), nu, self%p)
  if (.not. ok) call rp_die(self, "t")
end subroutine

subroutine tf_vec(self, x, nu)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:)
  real(c_float), intent(in) :: nu
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "t")
  ok = crp_tf(x, int(size(x), c_size_t), nu, self%p)
  if (.not. ok) call rp_die(self, "t")
end subroutine

subroutine tf_mat(self, x, nu)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:,:)
  real(c_float), intent(in) :: nu
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "t")
  ok = crp_tf(x, int(size(x), c_size_t), nu, self%p)
  if (.not. ok) call rp_die(self, "t")
end subroutine

subroutine f_vec(self, x, nu1, nu2)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:)
  real(c_double), intent(in) :: nu1
  real(c_double), intent(in) :: nu2
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "f")
  ok = crp_f(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. ok) call rp_die(self, "f")
end subroutine

subroutine f_mat(self, x, nu1, nu2)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:,:)
  real(c_double), intent(in) :: nu1
  real(c_double), intent(in) :: nu2
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "f")
  ok = crp_f(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. ok) call rp_die(self, "f")
end subroutine

subroutine ff_vec(self, x, nu1, nu2)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:)
  real(c_float), intent(in) :: nu1
  real(c_float), intent(in) :: nu2
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "f")
  ok = crp_ff(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. ok) call rp_die(self, "f")
end subroutine

subroutine ff_mat(self, x, nu1, nu2)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:,:)
  real(c_float), intent(in) :: nu1
  real(c_float), intent(in) :: nu2
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "f")
  ok = crp_ff(x, int(size(x), c_size_t), nu1, nu2, self%p)
  if (.not. ok) call rp_die(self, "f")
end subroutine

subroutine gumbel_vec(self, x, mu, beta)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:)
  real(c_double), intent(in) :: mu
  real(c_double), intent(in) :: beta
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "gumbel")
  ok = crp_gumbel(x, int(size(x), c_size_t), mu, beta, self%p)
  if (.not. ok) call rp_die(self, "gumbel")
end subroutine

subroutine gumbel_mat(self, x, mu, beta)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:,:)
  real(c_double), intent(in) :: mu
  real(c_double), intent(in) :: beta
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "gumbel")
  ok = crp_gumbel(x, int(size(x), c_size_t), mu, beta, self%p)
  if (.not. ok) call rp_die(self, "gumbel")
end subroutine

subroutine gumbelf_vec(self, x, mu, beta)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:)
  real(c_float), intent(in) :: mu
  real(c_float), intent(in) :: beta
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "gumbel")
  ok = crp_gumbelf(x, int(size(x), c_size_t), mu, beta, self%p)
  if (.not. ok) call rp_die(self, "gumbel")
end subroutine

subroutine gumbelf_mat(self, x, mu, beta)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:,:)
  real(c_float), intent(in) :: mu
  real(c_float), intent(in) :: beta
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "gumbel")
  ok = crp_gumbelf(x, int(size(x), c_size_t), mu, beta, self%p)
  if (.not. ok) call rp_die(self, "gumbel")
end subroutine

subroutine pareto_vec(self, x, xm, alpha)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:)
  real(c_double), intent(in) :: xm
  real(c_double), intent(in) :: alpha
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "pareto")
  ok = crp_pareto(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. ok) call rp_die(self, "pareto")
end subroutine

subroutine pareto_mat(self, x, xm, alpha)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:,:)
  real(c_double), intent(in) :: xm
  real(c_double), intent(in) :: alpha
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "pareto")
  ok = crp_pareto(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. ok) call rp_die(self, "pareto")
end subroutine

subroutine paretof_vec(self, x, xm, alpha)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:)
  real(c_float), intent(in) :: xm
  real(c_float), intent(in) :: alpha
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "pareto")
  ok = crp_paretof(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. ok) call rp_die(self, "pareto")
end subroutine

subroutine paretof_mat(self, x, xm, alpha)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:,:)
  real(c_float), intent(in) :: xm
  real(c_float), intent(in) :: alpha
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "pareto")
  ok = crp_paretof(x, int(size(x), c_size_t), xm, alpha, self%p)
  if (.not. ok) call rp_die(self, "pareto")
end subroutine

subroutine weibull_vec(self, x, shape, scale)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:)
  real(c_double), intent(in) :: shape
  real(c_double), intent(in) :: scale
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "weibull")
  ok = crp_weibull(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. ok) call rp_die(self, "weibull")
end subroutine

subroutine weibull_mat(self, x, shape, scale)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:,:)
  real(c_double), intent(in) :: shape
  real(c_double), intent(in) :: scale
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "weibull")
  ok = crp_weibull(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. ok) call rp_die(self, "weibull")
end subroutine

subroutine weibullf_vec(self, x, shape, scale)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:)
  real(c_float), intent(in) :: shape
  real(c_float), intent(in) :: scale
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "weibull")
  ok = crp_weibullf(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. ok) call rp_die(self, "weibull")
end subroutine

subroutine weibullf_mat(self, x, shape, scale)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:,:)
  real(c_float), intent(in) :: shape
  real(c_float), intent(in) :: scale
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "weibull")
  ok = crp_weibullf(x, int(size(x), c_size_t), shape, scale, self%p)
  if (.not. ok) call rp_die(self, "weibull")
end subroutine

subroutine skew_normal_vec(self, x, mu, sigma, alpha)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:)
  real(c_double), intent(in) :: mu
  real(c_double), intent(in) :: sigma
  real(c_double), intent(in) :: alpha
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "skew_normal")
  ok = crp_skew_normal(x, int(size(x), c_size_t), mu, sigma, alpha, self%p)
  if (.not. ok) call rp_die(self, "skew_normal")
end subroutine

subroutine skew_normal_mat(self, x, mu, sigma, alpha)
  class(rng), intent(inout) :: self
  real(c_double), intent(out) :: x(:,:)
  real(c_double), intent(in) :: mu
  real(c_double), intent(in) :: sigma
  real(c_double), intent(in) :: alpha
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "skew_normal")
  ok = crp_skew_normal(x, int(size(x), c_size_t), mu, sigma, alpha, self%p)
  if (.not. ok) call rp_die(self, "skew_normal")
end subroutine

subroutine skew_normalf_vec(self, x, mu, sigma, alpha)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:)
  real(c_float), intent(in) :: mu
  real(c_float), intent(in) :: sigma
  real(c_float), intent(in) :: alpha
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "skew_normal")
  ok = crp_skew_normalf(x, int(size(x), c_size_t), mu, sigma, alpha, self%p)
  if (.not. ok) call rp_die(self, "skew_normal")
end subroutine

subroutine skew_normalf_mat(self, x, mu, sigma, alpha)
  class(rng), intent(inout) :: self
  real(c_float), intent(out) :: x(:,:)
  real(c_float), intent(in) :: mu
  real(c_float), intent(in) :: sigma
  real(c_float), intent(in) :: alpha
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "skew_normal")
  ok = crp_skew_normalf(x, int(size(x), c_size_t), mu, sigma, alpha, self%p)
  if (.not. ok) call rp_die(self, "skew_normal")
end subroutine

subroutine int_vec(self, x, m, n)
  class(rng), intent(inout) :: self
  integer(c_int), intent(out) :: x(:)
  integer(c_int), intent(in) :: m
  integer(c_int), intent(in) :: n
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "int")
  ok = crp_int(x, int(size(x), c_size_t), m, n, self%p)
  if (.not. ok) call rp_die(self, "int")
end subroutine

subroutine int_mat(self, x, m, n)
  class(rng), intent(inout) :: self
  integer(c_int), intent(out) :: x(:,:)
  integer(c_int), intent(in) :: m
  integer(c_int), intent(in) :: n
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "int")
  ok = crp_int(x, int(size(x), c_size_t), m, n, self%p)
  if (.not. ok) call rp_die(self, "int")
end subroutine

subroutine rp_serialize(self, buf)
  class(rng), intent(inout) :: self
  integer(c_int8_t), allocatable, intent(out), target :: buf(:)
  integer(c_int) :: len
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "serialize")
  len = 0_c_int
  ok = crp_serialize(c_null_ptr, len, self%p)
  if (.not. ok) call rp_die(self, "serialize")
  if (len <= 0_c_int) then
    allocate(buf(0))
    return
  end if
  allocate(buf(len))
  ok = crp_serialize(c_loc(buf(1)), len, self%p)
  if (.not. ok) call rp_die(self, "serialize")
end subroutine

subroutine rp_deserialize(self, buf)
  class(rng), intent(inout) :: self
  integer(c_int8_t), intent(in), target :: buf(:)
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "deserialize")
  if (size(buf) == 0) call rp_die(self, "deserialize")
  ok = crp_deserialize(c_loc(buf(1)), int(size(buf), c_int), self%p)
  if (.not. ok) call rp_die(self, "deserialize")
end subroutine

subroutine rp_set_state(self, state)
  class(rng), intent(inout) :: self
  integer(c_int64_t), intent(in), target :: state(:)
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "set_state")
  if (size(state) <= 0) call rp_die(self, "set_state")
  ok = crp_set_state(state, int(size(state), c_int), self%p)
  if (.not. ok) call rp_die(self, "set_state")
end subroutine

subroutine rp_philox_set_state(self, ctr, key)
  class(rng), intent(inout) :: self
  type(randompack_philox_ctr), intent(in) :: ctr
  type(randompack_philox_key), intent(in) :: key
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "philox_set_state")
  ok = crp_philox_set_state(ctr, key, self%p)
  if (.not. ok) call rp_die(self, "philox_set_state")
end subroutine

subroutine rp_squares_set_state(self, ctr, key)
  class(rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: ctr
  integer(c_int64_t), intent(in) :: key
  logical(c_bool) :: ok
  if (.not. c_associated(self%p)) call rp_die(self, "squares_set_state")
  ok = crp_squares_set_state(ctr, key, self%p)
  if (.not. ok) call rp_die(self, "squares_set_state")
end subroutine

function rp_last_error(self) result(msg)
  class(rng), intent(in) :: self
  character(len=:), allocatable :: msg
  type(c_ptr) :: p
  if (.not. c_associated(self%p)) then
    msg = ""
    return
  end if
  p = crp_last_error(self%p)
  msg = cstr_to_fstring(p)
end function
