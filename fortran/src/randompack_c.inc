
interface
  function crp_create(engine) bind(C, name="randompack_create") result(p)
    import :: c_ptr
    type(c_ptr), value :: engine
    type(c_ptr) :: p
  end function

  function crp_duplicate(rngp) bind(C, name="randompack_duplicate") result(p)
    import :: c_ptr
    type(c_ptr), value :: rngp
    type(c_ptr) :: p
  end function

  logical(c_bool) function crp_randomize(rngp) bind(C, name="randompack_randomize")
    import :: c_ptr, c_bool
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_full_mantissa(rngp, enable) bind(C, name="randompack_full_mantissa")
    import :: c_ptr, c_bool
    type(c_ptr), value :: rngp
    logical(c_bool), value :: enable
  end function

  logical(c_bool) function crp_bitexact(rngp, enable) bind(C, name="randompack_bitexact")
    import :: c_ptr, c_bool
    type(c_ptr), value :: rngp
    logical(c_bool), value :: enable
  end function

  logical(c_bool) function crp_jump(p, rngp) bind(C, name="randompack_jump")
    import :: c_int, c_ptr, c_bool
    integer(c_int), value :: p
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_seed(seed, spawn_key, n_key, rngp) bind(C, name="randompack_seed")
    import :: c_int32_t, c_ptr, c_bool, c_int
    integer(c_int32_t), value :: seed
    type(c_ptr), value :: spawn_key
    integer(c_int), value :: n_key
    type(c_ptr), value :: rngp
  end function

  subroutine crp_free(rngp) bind(C, name="randompack_free")
    import :: c_ptr
    type(c_ptr), value :: rngp
  end subroutine

  logical(c_bool) function crp_u01(x, n, rngp) bind(C, name="randompack_u01")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_u01f(x, n, rngp) bind(C, name="randompack_u01f")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_unif(x, n, a, b, rngp) bind(C, name="randompack_unif")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: a
    real(c_double), value :: b
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_uniff(x, n, a, b, rngp) bind(C, name="randompack_uniff")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: a
    real(c_float), value :: b
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_norm(x, n, rngp) bind(C, name="randompack_norm")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_normf(x, n, rngp) bind(C, name="randompack_normf")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_normal(x, n, mu, sigma, rngp) bind(C, name="randompack_normal")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: mu
    real(c_double), value :: sigma
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_normalf(x, n, mu, sigma, rngp) bind(C, name="randompack_normalf")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: mu
    real(c_float), value :: sigma
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_exp(x, n, scale, rngp) bind(C, name="randompack_exp")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: scale
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_expf(x, n, scale, rngp) bind(C, name="randompack_expf")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: scale
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_lognormal(x, n, mu, sigma, rngp) bind(C, name="randompack_lognormal")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: mu
    real(c_double), value :: sigma
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_lognormalf(x, n, mu, sigma, rngp) bind(C, name="randompack_lognormalf")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: mu
    real(c_float), value :: sigma
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_gamma(x, n, shape, scale, rngp) bind(C, name="randompack_gamma")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: shape
    real(c_double), value :: scale
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_gammaf(x, n, shape, scale, rngp) bind(C, name="randompack_gammaf")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: shape
    real(c_float), value :: scale
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_beta(x, n, a, b, rngp) bind(C, name="randompack_beta")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: a
    real(c_double), value :: b
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_betaf(x, n, a, b, rngp) bind(C, name="randompack_betaf")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: a
    real(c_float), value :: b
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_chi2(x, n, nu, rngp) bind(C, name="randompack_chi2")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: nu
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_chi2f(x, n, nu, rngp) bind(C, name="randompack_chi2f")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: nu
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_t(x, n, nu, rngp) bind(C, name="randompack_t")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: nu
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_tf(x, n, nu, rngp) bind(C, name="randompack_tf")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: nu
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_f(x, n, nu1, nu2, rngp) bind(C, name="randompack_f")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: nu1
    real(c_double), value :: nu2
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_ff(x, n, nu1, nu2, rngp) bind(C, name="randompack_ff")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: nu1
    real(c_float), value :: nu2
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_gumbel(x, n, mu, beta, rngp) bind(C, name="randompack_gumbel")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: mu
    real(c_double), value :: beta
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_gumbelf(x, n, mu, beta, rngp) bind(C, name="randompack_gumbelf")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: mu
    real(c_float), value :: beta
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_pareto(x, n, xm, alpha, rngp) bind(C, name="randompack_pareto")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: xm
    real(c_double), value :: alpha
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_paretof(x, n, xm, alpha, rngp) bind(C, name="randompack_paretof")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: xm
    real(c_float), value :: alpha
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_weibull(x, n, shape, scale, rngp) bind(C, name="randompack_weibull")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: shape
    real(c_double), value :: scale
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_weibullf(x, n, shape, scale, rngp) bind(C, name="randompack_weibullf")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: shape
    real(c_float), value :: scale
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_skew_normal(x, n, mu, sigma, alpha, rngp) &
    bind(C, name="randompack_skew_normal")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: mu
    real(c_double), value :: sigma
    real(c_double), value :: alpha
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_skew_normalf(x, n, mu, sigma, alpha, rngp) &
    bind(C, name="randompack_skew_normalf")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: mu
    real(c_float), value :: sigma
    real(c_float), value :: alpha
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_int(x, len, m, n, rngp) bind(C, name="randompack_int")
    import :: c_int, c_int32_t, c_size_t, c_ptr, c_bool
    integer(c_int32_t) :: x(*)
    integer(c_size_t), value :: len
    integer(c_int32_t), value :: m
    integer(c_int32_t), value :: n
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_long_long(x, len, m, n, rngp) &
    bind(C, name="randompack_long_long")
    import :: c_int64_t, c_size_t, c_ptr, c_bool
    integer(c_int64_t) :: x(*)
    integer(c_size_t), value :: len
    integer(c_int64_t), value :: m
    integer(c_int64_t), value :: n
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_uint32(x, len, bound, rngp) &
    bind(C, name="randompack_uint32")
    import :: c_int32_t, c_size_t, c_ptr, c_bool
    integer(c_int32_t) :: x(*)
    integer(c_size_t), value :: len
    integer(c_int32_t), value :: bound
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_uint64(x, len, bound, rngp) &
    bind(C, name="randompack_uint64")
    import :: c_int64_t, c_size_t, c_ptr, c_bool
    integer(c_int64_t) :: x(*)
    integer(c_size_t), value :: len
    integer(c_int64_t), value :: bound
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_engines(eng, desc, neng, eng_maxlen, desc_maxlen) bind(C, name="randompack_engines")
    import :: c_ptr, c_int, c_bool
    type(c_ptr), value :: eng
    type(c_ptr), value :: desc
    integer(c_int) :: neng
    integer(c_int) :: eng_maxlen
    integer(c_int) :: desc_maxlen
  end function

  logical(c_bool) function crp_serialize(buf, len, rngp) bind(C, name="randompack_serialize")
    import :: c_ptr, c_int, c_bool
    type(c_ptr), value :: buf
    integer(c_int) :: len
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_deserialize(buf, len, rngp) bind(C, name="randompack_deserialize")
    import :: c_ptr, c_int, c_bool
    type(c_ptr), value :: buf
    integer(c_int), value :: len
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_philox_set_key(key, rngp) bind(C, name="randompack_philox_set_key")
    import :: c_ptr, c_bool, c_int64_t
    integer(c_int64_t), intent(in) :: key(2)
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_squares_set_key(key, rngp) bind(C, name="randompack_squares_set_key")
    import :: c_ptr, c_bool, c_int64_t
    integer(c_int64_t), value :: key
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_pcg64_set_inc(inc, rngp) bind(C, name="randompack_pcg64_set_inc")
    import :: c_ptr, c_bool, c_int64_t
    integer(c_int64_t), intent(in) :: inc(2)
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_sfc64_set_abc(abc, rngp) bind(C, name="randompack_sfc64_set_abc")
    import :: c_ptr, c_bool, c_int64_t
    integer(c_int64_t), intent(in) :: abc(3)
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_set_chacha_nonce(nonce, rngp) bind(C, name="randompack_set_chacha_nonce")
    import :: c_ptr, c_bool, c_int32_t
    integer(c_int32_t), intent(in) :: nonce(3)
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_set_state(state, nstate, rngp) bind(C, name="randompack_set_state")
    import :: c_ptr, c_bool, c_int64_t, c_int
    integer(c_int64_t) :: state(*)
    integer(c_int), value :: nstate
    type(c_ptr), value :: rngp
  end function

  function crp_last_error(rngp) bind(C, name="randompack_last_error") result(p)
    import :: c_ptr
    type(c_ptr), value :: rngp
    type(c_ptr) :: p
  end function

end interface
