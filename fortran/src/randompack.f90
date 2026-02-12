module randompack
use, intrinsic :: iso_c_binding, only: c_ptr, c_char, c_bool, c_int, &
  c_size_t, c_double, c_float, c_int8_t, c_int32_t, c_int64_t, c_null_ptr, &
  c_null_char, c_associated, c_loc, c_f_pointer
implicit none
private
public :: randompack_rng, engines

integer, parameter :: MAXSTRLEN = 1000

type, bind(C) :: c_philox_ctr
  integer(c_int64_t) :: v(4)
end type

type, bind(C) :: c_philox_key
  integer(c_int64_t) :: v(2)
end type

type, public :: randompack_philox_ctr
  integer(c_int64_t) :: v(4)
end type

type, public :: randompack_philox_key
  integer(c_int64_t) :: v(2)
end type

type :: randompack_rng
  type(c_ptr) :: p = c_null_ptr
contains
  procedure :: create => rp_create
  procedure :: free => rp_free
  procedure :: duplicate => rp_duplicate
  procedure :: randomize => rp_randomize
  procedure :: full_mantissa => rp_full_mantissa
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
  procedure :: philox_set_state => rp_philox_set_state
  procedure, private :: squares_set_state32 => rp_squares_set_state32
  procedure, private :: squares_set_state64 => rp_squares_set_state64
  generic :: squares_set_state => squares_set_state32, squares_set_state64
  procedure :: last_error => rp_last_error
  final :: rp_finalize
end type randompack_rng

include "randompack_c.inc"

contains
include "util.inc"
include "methods.inc"

end module randompack
