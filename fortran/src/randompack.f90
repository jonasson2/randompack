module randompack
use, intrinsic :: iso_c_binding, only: c_ptr, c_char, c_bool, c_int, &
  c_size_t, c_double, c_float, c_int8_t, c_int32_t, c_int64_t, c_null_ptr, &
  c_null_char, c_associated, c_loc, c_f_pointer
implicit none
private
public :: randompack_rng, engines

integer, parameter :: MAXSTRLEN = 1000

type :: randompack_rng
  type(c_ptr) :: p = c_null_ptr
contains
  procedure :: create => rp_create
  procedure :: free => rp_free
  procedure :: duplicate => rp_duplicate
  procedure :: randomize => rp_randomize
  procedure :: jump => rp_jump
  procedure, private :: seed32 => rp_seed32
  procedure, private :: seed64 => rp_seed64
  generic :: seed => seed32, seed64
  procedure, private :: unif_vec
  procedure, private :: unif_mat
  procedure, private :: uniff_vec
  procedure, private :: uniff_mat
  generic :: unif => unif_vec, unif_mat, uniff_vec, uniff_mat
  procedure, private :: normal_vec
  procedure, private :: normal_mat
  procedure, private :: normalf_vec
  procedure, private :: normalf_mat
  generic :: normal => normal_vec, normal_mat, normalf_vec, normalf_mat
  procedure, private :: exp_vec
  procedure, private :: exp_mat
  procedure, private :: expf_vec
  procedure, private :: expf_mat
  generic :: exp => exp_vec, exp_mat, expf_vec, expf_mat
  procedure, private :: lognormal_vec
  procedure, private :: lognormal_mat
  procedure, private :: lognormalf_vec
  procedure, private :: lognormalf_mat
  generic :: lognormal => lognormal_vec, lognormal_mat, lognormalf_vec, lognormalf_mat
  procedure, private :: gamma_vec
  procedure, private :: gamma_mat
  procedure, private :: gammaf_vec
  procedure, private :: gammaf_mat
  generic :: gamma => gamma_vec, gamma_mat, gammaf_vec, gammaf_mat
  procedure, private :: beta_vec
  procedure, private :: beta_mat
  procedure, private :: betaf_vec
  procedure, private :: betaf_mat
  generic :: beta => beta_vec, beta_mat, betaf_vec, betaf_mat
  procedure, private :: chi2_vec
  procedure, private :: chi2_mat
  procedure, private :: chi2f_vec
  procedure, private :: chi2f_mat
  generic :: chi2 => chi2_vec, chi2_mat, chi2f_vec, chi2f_mat
  procedure, private :: t_vec
  procedure, private :: t_mat
  procedure, private :: tf_vec
  procedure, private :: tf_mat
  generic :: t => t_vec, t_mat, tf_vec, tf_mat
  procedure, private :: f_vec
  procedure, private :: f_mat
  procedure, private :: ff_vec
  procedure, private :: ff_mat
  generic :: f => f_vec, f_mat, ff_vec, ff_mat
  procedure, private :: gumbel_vec
  procedure, private :: gumbel_mat
  procedure, private :: gumbelf_vec
  procedure, private :: gumbelf_mat
  generic :: gumbel => gumbel_vec, gumbel_mat, gumbelf_vec, gumbelf_mat
  procedure, private :: pareto_vec
  procedure, private :: pareto_mat
  procedure, private :: paretof_vec
  procedure, private :: paretof_mat
  generic :: pareto => pareto_vec, pareto_mat, paretof_vec, paretof_mat
  procedure, private :: weibull_vec
  procedure, private :: weibull_mat
  procedure, private :: weibullf_vec
  procedure, private :: weibullf_mat
  generic :: weibull => weibull_vec, weibull_mat, weibullf_vec, weibullf_mat
  procedure, private :: skew_normal_vec
  procedure, private :: skew_normal_mat
  procedure, private :: skew_normalf_vec
  procedure, private :: skew_normalf_mat
  generic :: skew_normal => skew_normal_vec, skew_normal_mat, &
    skew_normalf_vec, skew_normalf_mat
  procedure :: mvn => mvn_mat
  procedure, private :: int32_vec
  procedure, private :: int32_mat
  procedure, private :: int64_vec
  procedure, private :: int64_mat
  procedure, private :: int64_vec32
  procedure, private :: int64_mat32
  generic :: int => int32_vec, int32_mat, int64_vec, int64_mat, &
    int64_vec32, int64_mat32
  procedure, private :: perm32_vec
  procedure, private :: perm64_vec
  generic :: perm => perm32_vec, perm64_vec
  procedure, private :: sample32_vec
  procedure, private :: sample32_vec64
  procedure, private :: sample64_vec
  procedure, private :: sample64_vec32
  generic :: sample => sample32_vec, sample32_vec64, sample64_vec, sample64_vec32
  procedure, private :: raw32_vec
  procedure, private :: raw32_mat
  procedure, private :: raw64_vec
  procedure, private :: raw64_mat
  generic :: raw => raw32_vec, raw32_mat, raw64_vec, raw64_mat
  procedure :: serialize => rp_serialize
  procedure :: deserialize => rp_deserialize
  procedure, private :: set_state32
  procedure, private :: set_state64
  generic :: set_state => set_state32, set_state64
  procedure, private :: advance32 => rp_advance32
  procedure, private :: advance64 => rp_advance64
  procedure, private :: advance128 => rp_advance128
  generic :: advance => advance32, advance64, advance128
  procedure :: philox_set_key => rp_philox_set_key
  procedure :: pcg64_set_inc => rp_pcg64_set_inc
  procedure :: sfc64_set_abc => rp_sfc64_set_abc
  procedure :: chacha_set_nonce => rp_chacha_set_nonce
  procedure, private :: squares_set_key32 => rp_squares_set_key32
  procedure, private :: squares_set_key64 => rp_squares_set_key64
  generic :: squares_set_key => squares_set_key32, squares_set_key64
  procedure :: last_error => rp_last_error
  final :: rp_finalize
end type randompack_rng

! C bindings to the underlying library.
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

  logical(c_bool) function crp_full_mantissa(rngp, enable) &
    bind(C, name="randompack_full_mantissa")
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

  logical(c_bool) function crp_seed(seed, spawn_key, n_key, rngp) &
    bind(C, name="randompack_seed")
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

  logical(c_bool) function crp_normal(x, n, mu, sigma, rngp) &
    bind(C, name="randompack_normal")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: mu
    real(c_double), value :: sigma
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_normalf(x, n, mu, sigma, rngp) &
    bind(C, name="randompack_normalf")
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

  logical(c_bool) function crp_lognormal(x, n, mu, sigma, rngp) &
    bind(C, name="randompack_lognormal")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: mu
    real(c_double), value :: sigma
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_lognormalf(x, n, mu, sigma, rngp) &
    bind(C, name="randompack_lognormalf")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: mu
    real(c_float), value :: sigma
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_gamma(x, n, shape, scale, rngp) &
    bind(C, name="randompack_gamma")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: shape
    real(c_double), value :: scale
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_gammaf(x, n, shape, scale, rngp) &
    bind(C, name="randompack_gammaf")
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

  logical(c_bool) function crp_gumbel(x, n, mu, beta, rngp) &
    bind(C, name="randompack_gumbel")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: mu
    real(c_double), value :: beta
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_gumbelf(x, n, mu, beta, rngp) &
    bind(C, name="randompack_gumbelf")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: mu
    real(c_float), value :: beta
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_pareto(x, n, xm, alpha, rngp) &
    bind(C, name="randompack_pareto")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: xm
    real(c_double), value :: alpha
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_paretof(x, n, xm, alpha, rngp) &
    bind(C, name="randompack_paretof")
    import :: c_float, c_size_t, c_ptr, c_bool
    real(c_float) :: x(*)
    integer(c_size_t), value :: n
    real(c_float), value :: xm
    real(c_float), value :: alpha
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_weibull(x, n, shape, scale, rngp) &
    bind(C, name="randompack_weibull")
    import :: c_double, c_size_t, c_ptr, c_bool
    real(c_double) :: x(*)
    integer(c_size_t), value :: n
    real(c_double), value :: shape
    real(c_double), value :: scale
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_weibullf(x, n, shape, scale, rngp) &
    bind(C, name="randompack_weibullf")
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

  logical(c_bool) function crp_mvn(transp, mu, Sig, d, n, X, ldx, L, rngp) &
    bind(C, name="randompack_mvn")
    import :: c_ptr, c_bool, c_int, c_size_t
    type(c_ptr), value :: transp
    type(c_ptr), value :: mu
    type(c_ptr), value :: Sig
    integer(c_int), value :: d
    integer(c_size_t), value :: n
    type(c_ptr), value :: X
    integer(c_int), value :: ldx
    type(c_ptr), value :: L
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

  logical(c_bool) function crp_perm(x, len, rngp) bind(C, name="randompack_perm")
    import :: c_int, c_int32_t, c_ptr, c_bool
    integer(c_int32_t) :: x(*)
    integer(c_int), value :: len
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_sample(x, len, k, rngp) bind(C, name="randompack_sample")
    import :: c_int, c_int32_t, c_ptr, c_bool
    integer(c_int32_t) :: x(*)
    integer(c_int), value :: len
    integer(c_int), value :: k
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

  logical(c_bool) function crp_engines(eng, desc, neng, eng_maxlen, desc_maxlen) &
    bind(C, name="randompack_engines")
    import :: c_ptr, c_int, c_bool
    type(c_ptr), value :: eng
    type(c_ptr), value :: desc
    integer(c_int) :: neng
    integer(c_int) :: eng_maxlen
    integer(c_int) :: desc_maxlen
  end function

  logical(c_bool) function crp_serialize(buf, len, rngp) &
    bind(C, name="randompack_serialize")
    import :: c_ptr, c_int, c_bool
    type(c_ptr), value :: buf
    integer(c_int) :: len
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_deserialize(buf, len, rngp) &
    bind(C, name="randompack_deserialize")
    import :: c_ptr, c_int, c_bool
    type(c_ptr), value :: buf
    integer(c_int), value :: len
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_philox_set_key(key, rngp) &
    bind(C, name="randompack_philox_set_key")
    import :: c_ptr, c_bool, c_int64_t
    integer(c_int64_t), intent(in) :: key(2)
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_advance(delta, rngp) bind(C, name="randompack_advance")
    import :: c_ptr, c_bool, c_int64_t
    integer(c_int64_t), intent(in) :: delta(2)
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_squares_set_key(key, rngp) &
    bind(C, name="randompack_squares_set_key")
    import :: c_ptr, c_bool, c_int64_t
    integer(c_int64_t), value :: key
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_pcg64_set_inc(inc, rngp) &
    bind(C, name="randompack_pcg64_set_inc")
    import :: c_ptr, c_bool, c_int64_t
    integer(c_int64_t), intent(in) :: inc(2)
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_sfc64_set_abc(abc, rngp) &
    bind(C, name="randompack_sfc64_set_abc")
    import :: c_ptr, c_bool, c_int64_t
    integer(c_int64_t), intent(in) :: abc(3)
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_chacha_set_nonce(nonce, rngp) &
    bind(C, name="randompack_chacha_set_nonce")
    import :: c_ptr, c_bool, c_int32_t
    integer(c_int32_t), intent(in) :: nonce(3)
    type(c_ptr), value :: rngp
  end function

  logical(c_bool) function crp_set_state(state, nstate, rngp) &
    bind(C, name="randompack_set_state")
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

! Helper routines and free procedures implemented in the submodule.
interface
  pure module function to_c_string(s) result(cs)
  character(len=*), intent(in) :: s
  character(kind=c_char), allocatable :: cs(:)
  end function

  module function cstr_to_fstring(p) result(msg)
  type(c_ptr), intent(in) :: p
  character(len=:), allocatable :: msg
  end function

  module subroutine check_kinds()
  end subroutine

  module function rp_error_message(self, where) result(msg)
  class(randompack_rng), intent(in) :: self
  character(len=*), intent(in) :: where
  character(len=:), allocatable :: msg
  end function

  module subroutine engines(names, descriptions)
  character(len=:), allocatable, intent(out) :: names(:)
  character(len=:), allocatable, intent(out) :: descriptions(:)
  end subroutine
end interface

! Type-bound procedure implementations provided by the submodule.
interface
  module subroutine rp_finalize(self)
  type(randompack_rng), intent(inout) :: self
  end subroutine

  module subroutine rp_free(self)
  class(randompack_rng), intent(inout) :: self
  end subroutine

  module subroutine rp_create(self, engine, bitexact, full_mantissa)
  class(randompack_rng), intent(inout) :: self
  character(len=*), intent(in), optional :: engine
  logical, intent(in), optional :: bitexact
  logical, intent(in), optional :: full_mantissa
  end subroutine

  module subroutine rp_duplicate(self, out)
  class(randompack_rng), intent(in) :: self
  type(randompack_rng), intent(out) :: out
  end subroutine

  module subroutine rp_randomize(self)
  class(randompack_rng), intent(inout) :: self
  end subroutine

  module subroutine rp_jump(self, p)
  class(randompack_rng), intent(inout) :: self
  integer(c_int), intent(in) :: p
  end subroutine

  module subroutine rp_seed32(self, seed, spawn_key)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(in) :: seed
  integer(c_int32_t), intent(in), optional, target :: spawn_key(:)
  end subroutine

  module subroutine rp_seed64(self, seed, spawn_key)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: seed
  integer(c_int64_t), intent(in), optional, target :: spawn_key(:)
  end subroutine

  module subroutine unif_vec(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in), optional :: a
  double precision, intent(in), optional :: b
  end subroutine

  module subroutine unif_mat(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in), optional :: a
  double precision, intent(in), optional :: b
  end subroutine

  module subroutine uniff_vec(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in), optional :: a
  real, intent(in), optional :: b
  end subroutine

  module subroutine uniff_mat(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in), optional :: a
  real, intent(in), optional :: b
  end subroutine

  module subroutine normal_vec(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: sigma
  end subroutine

  module subroutine normal_mat(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: sigma
  end subroutine

  module subroutine normalf_vec(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: sigma
  end subroutine

  module subroutine normalf_mat(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: sigma
  end subroutine

  module subroutine exp_vec(self, x, scale)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in), optional :: scale
  end subroutine

  module subroutine exp_mat(self, x, scale)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in), optional :: scale
  end subroutine

  module subroutine expf_vec(self, x, scale)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in), optional :: scale
  end subroutine

  module subroutine expf_mat(self, x, scale)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in), optional :: scale
  end subroutine

  module subroutine lognormal_vec(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: sigma
  end subroutine

  module subroutine lognormal_mat(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: sigma
  end subroutine

  module subroutine lognormalf_vec(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: sigma
  end subroutine

  module subroutine lognormalf_mat(self, x, mu, sigma)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: sigma
  end subroutine

  module subroutine gamma_vec(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: shape
  double precision, intent(in), optional :: scale
  end subroutine

  module subroutine gamma_mat(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: shape
  double precision, intent(in), optional :: scale
  end subroutine

  module subroutine gammaf_vec(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: shape
  real, intent(in), optional :: scale
  end subroutine

  module subroutine gammaf_mat(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: shape
  real, intent(in), optional :: scale
  end subroutine

  module subroutine beta_vec(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: a
  double precision, intent(in) :: b
  end subroutine

  module subroutine beta_mat(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: a
  double precision, intent(in) :: b
  end subroutine

  module subroutine betaf_vec(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: a
  real, intent(in) :: b
  end subroutine

  module subroutine betaf_mat(self, x, a, b)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: a
  real, intent(in) :: b
  end subroutine

  module subroutine chi2_vec(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: nu
  end subroutine

  module subroutine chi2_mat(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: nu
  end subroutine

  module subroutine chi2f_vec(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: nu
  end subroutine

  module subroutine chi2f_mat(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: nu
  end subroutine

  module subroutine t_vec(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: nu
  end subroutine

  module subroutine t_mat(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: nu
  end subroutine

  module subroutine tf_vec(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: nu
  end subroutine

  module subroutine tf_mat(self, x, nu)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: nu
  end subroutine

  module subroutine f_vec(self, x, nu1, nu2)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: nu1
  double precision, intent(in) :: nu2
  end subroutine

  module subroutine f_mat(self, x, nu1, nu2)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: nu1
  double precision, intent(in) :: nu2
  end subroutine

  module subroutine ff_vec(self, x, nu1, nu2)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: nu1
  real, intent(in) :: nu2
  end subroutine

  module subroutine ff_mat(self, x, nu1, nu2)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: nu1
  real, intent(in) :: nu2
  end subroutine

  module subroutine gumbel_vec(self, x, mu, beta)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: beta
  end subroutine

  module subroutine gumbel_mat(self, x, mu, beta)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: beta
  end subroutine

  module subroutine gumbelf_vec(self, x, mu, beta)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: beta
  end subroutine

  module subroutine gumbelf_mat(self, x, mu, beta)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: beta
  end subroutine

  module subroutine pareto_vec(self, x, xm, alpha)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: xm
  double precision, intent(in) :: alpha
  end subroutine

  module subroutine pareto_mat(self, x, xm, alpha)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: xm
  double precision, intent(in) :: alpha
  end subroutine

  module subroutine paretof_vec(self, x, xm, alpha)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: xm
  real, intent(in) :: alpha
  end subroutine

  module subroutine paretof_mat(self, x, xm, alpha)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: xm
  real, intent(in) :: alpha
  end subroutine

  module subroutine weibull_vec(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in) :: shape
  double precision, intent(in), optional :: scale
  end subroutine

  module subroutine weibull_mat(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in) :: shape
  double precision, intent(in), optional :: scale
  end subroutine

  module subroutine weibullf_vec(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in) :: shape
  real, intent(in), optional :: scale
  end subroutine

  module subroutine weibullf_mat(self, x, shape, scale)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in) :: shape
  real, intent(in), optional :: scale
  end subroutine

  module subroutine skew_normal_vec(self, x, mu, sigma, alpha)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: sigma
  double precision, intent(in) :: alpha
  end subroutine

  module subroutine skew_normal_mat(self, x, mu, sigma, alpha)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out) :: x(:,:)
  double precision, intent(in), optional :: mu
  double precision, intent(in), optional :: sigma
  double precision, intent(in) :: alpha
  end subroutine

  module subroutine skew_normalf_vec(self, x, mu, sigma, alpha)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: sigma
  real, intent(in) :: alpha
  end subroutine

  module subroutine skew_normalf_mat(self, x, mu, sigma, alpha)
  class(randompack_rng), intent(inout) :: self
  real, intent(out) :: x(:,:)
  real, intent(in), optional :: mu
  real, intent(in), optional :: sigma
  real, intent(in) :: alpha
  end subroutine

  module subroutine mvn_mat(self, x, Sigma, mu, trans)
  class(randompack_rng), intent(inout) :: self
  double precision, intent(out), target :: x(:,:)
  double precision, intent(in) :: Sigma(:,:)
  double precision, intent(in), optional :: mu(:)
  character(len=1), intent(in), optional :: trans
  end subroutine

  module subroutine int32_vec(self, x, m, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(out) :: x(:)
  integer(c_int32_t), intent(in) :: m
  integer(c_int32_t), intent(in) :: n
  end subroutine

  module subroutine int32_mat(self, x, m, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(out) :: x(:,:)
  integer(c_int32_t), intent(in) :: m
  integer(c_int32_t), intent(in) :: n
  end subroutine

  module subroutine int64_vec(self, x, m, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:)
  integer(c_int64_t), intent(in) :: m
  integer(c_int64_t), intent(in) :: n
  end subroutine

  module subroutine int64_mat(self, x, m, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:,:)
  integer(c_int64_t), intent(in) :: m
  integer(c_int64_t), intent(in) :: n
  end subroutine

  module subroutine int64_vec32(self, x, m, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:)
  integer(c_int32_t), intent(in) :: m
  integer(c_int32_t), intent(in) :: n
  end subroutine

  module subroutine int64_mat32(self, x, m, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:,:)
  integer(c_int32_t), intent(in) :: m
  integer(c_int32_t), intent(in) :: n
  end subroutine

  module subroutine perm32_vec(self, x)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(out) :: x(:)
  end subroutine

  module subroutine perm64_vec(self, x)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:)
  end subroutine

  module subroutine sample32_vec(self, x, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(out) :: x(:)
  integer(c_int32_t), intent(in) :: n
  end subroutine

  module subroutine sample32_vec64(self, x, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(out) :: x(:)
  integer(c_int64_t), intent(in) :: n
  end subroutine

  module subroutine sample64_vec(self, x, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:)
  integer(c_int64_t), intent(in) :: n
  end subroutine

  module subroutine sample64_vec32(self, x, n)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:)
  integer(c_int32_t), intent(in) :: n
  end subroutine

  module subroutine raw32_vec(self, x)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(out) :: x(:)
  end subroutine

  module subroutine raw32_mat(self, x)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(out) :: x(:,:)
  end subroutine

  module subroutine raw64_vec(self, x)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:)
  end subroutine

  module subroutine raw64_mat(self, x)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(out) :: x(:,:)
  end subroutine

  module subroutine rp_serialize(self, buf)
  class(randompack_rng), intent(inout) :: self
  integer(c_int8_t), allocatable, intent(out) :: buf(:)
  end subroutine

  module subroutine rp_deserialize(self, buf)
  class(randompack_rng), intent(inout) :: self
  integer(c_int8_t), intent(in) :: buf(:)
  end subroutine

  module subroutine set_state32(self, state)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(in) :: state(:)
  end subroutine

  module subroutine set_state64(self, state)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in), target :: state(:)
  end subroutine

  module subroutine rp_advance32(self, delta)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(in) :: delta
  end subroutine

  module subroutine rp_advance64(self, delta)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: delta
  end subroutine

  module subroutine rp_advance128(self, delta)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: delta(2)
  end subroutine

  module subroutine rp_philox_set_key(self, key)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: key(2)
  end subroutine

  module subroutine rp_pcg64_set_inc(self, inc)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: inc(2)
  end subroutine

  module subroutine rp_sfc64_set_abc(self, abc)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: abc(3)
  end subroutine

  module subroutine rp_chacha_set_nonce(self, nonce)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(in) :: nonce(3)
  end subroutine

  module subroutine rp_squares_set_key32(self, key)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(in) :: key
  end subroutine

  module subroutine rp_squares_set_key64(self, key)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: key
  end subroutine

  module function rp_last_error(self) result(msg)
  class(randompack_rng), intent(in) :: self
  character(len=:), allocatable :: msg
  end function
end interface

end module randompack
