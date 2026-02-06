module randompack
use, intrinsic :: iso_c_binding, only: c_ptr, c_char, c_bool, c_int, &
  c_size_t, c_double, c_float, c_int8_t, c_int64_t, c_null_ptr, c_null_char, &
  c_associated, c_loc, c_f_pointer
use, intrinsic :: iso_fortran_env, only: int64
implicit none
private
public :: rng, engines

type, bind(C) :: c_philox_ctr
  integer(c_int64_t) :: v(4)
end type

type, bind(C) :: c_philox_key
  integer(c_int64_t) :: v(2)
end type

type, public :: randompack_philox_ctr
  integer(int64) :: v(4)
end type

type, public :: randompack_philox_key
  integer(int64) :: v(2)
end type

type :: rng
  type(c_ptr) :: p = c_null_ptr
contains
  procedure :: create => rp_create
  procedure :: free => rp_free
  procedure :: duplicate => rp_duplicate
  procedure :: randomize => rp_randomize
  procedure :: full_mantissa => rp_full_mantissa
  procedure :: seed => rp_seed
  procedure, private :: u01_vec
  procedure, private :: u01_mat
  procedure, private :: u01f_vec
  procedure, private :: u01f_mat
  generic :: u01 => u01_vec, u01_mat, u01f_vec, u01f_mat
  procedure, private :: unif_vec
  procedure, private :: unif_mat
  procedure, private :: uniff_vec
  procedure, private :: uniff_mat
  generic :: unif => unif_vec, unif_mat, uniff_vec, uniff_mat
  procedure, private :: norm_vec
  procedure, private :: norm_mat
  procedure, private :: normf_vec
  procedure, private :: normf_mat
  generic :: norm => norm_vec, norm_mat, normf_vec, normf_mat
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
  procedure, private :: int_vec
  procedure, private :: int_mat
  generic :: int => int_vec, int_mat
  procedure :: serialize => rp_serialize
  procedure :: deserialize => rp_deserialize
  procedure :: set_state => rp_set_state
  procedure :: philox_set_state => rp_philox_set_state
  procedure :: squares_set_state => rp_squares_set_state
  procedure :: last_error => rp_last_error
  final :: rp_finalize
end type

include "randompack_c.inc"

contains
include "util.inc"
include "methods.inc"

end module randompack
