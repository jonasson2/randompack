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
  generic :: skew_normal => skew_normal_vec, skew_normal_mat, skew_normalf_vec, skew_normalf_mat
  procedure, private :: int32_vec
  procedure, private :: int32_mat
  procedure, private :: int64_vec
  procedure, private :: int64_mat
  procedure, private :: int64_vec32
  procedure, private :: int64_mat32
  generic :: int => int32_vec, int32_mat, int64_vec, int64_mat, &
    int64_vec32, int64_mat32
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

  logical(c_bool) function crp_advance(delta, rngp) bind(C, name="randompack_advance")
    import :: c_ptr, c_bool, c_int64_t
    integer(c_int64_t), intent(in) :: delta(2)
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

  logical(c_bool) function crp_chacha_set_nonce(nonce, rngp) bind(C, name="randompack_chacha_set_nonce")
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

contains
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

subroutine rp_create(self, engine, bitexact, full_mantissa)
  class(randompack_rng), intent(inout) :: self
  character(len=*), intent(in), optional :: engine
  logical, intent(in), optional :: bitexact
  logical, intent(in), optional :: full_mantissa
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

subroutine rp_advance32(self, delta)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(in) :: delta
  integer(c_int64_t) :: delta64(2)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "advance")
  if (delta < 0_c_int32_t) error stop "advance: delta must be nonnegative"
  delta64 = [int(delta, c_int64_t), 0_c_int64_t]
  c_ok = crp_advance(delta64, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "advance")
end subroutine

subroutine rp_advance64(self, delta)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: delta
  integer(c_int64_t) :: delta64(2)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "advance")
  if (delta < 0_c_int64_t) error stop "advance: delta must be nonnegative"
  delta64 = [delta, 0_c_int64_t]
  c_ok = crp_advance(delta64, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "advance")
end subroutine

subroutine rp_advance128(self, delta)
  class(randompack_rng), intent(inout) :: self
  integer(c_int64_t), intent(in) :: delta(2)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "advance")
  c_ok = crp_advance(delta, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "advance")
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

subroutine rp_chacha_set_nonce(self, nonce)
  class(randompack_rng), intent(inout) :: self
  integer(c_int32_t), intent(in) :: nonce(3)
  logical(c_bool) :: c_ok
  if (.not. c_associated(self%p)) error stop rp_error_message(self, "chacha_set_nonce")
  c_ok = crp_chacha_set_nonce(nonce, self%p)
  if (.not. c_ok) error stop rp_error_message(self, "chacha_set_nonce")
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

end module randompack
