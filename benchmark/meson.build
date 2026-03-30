common_inc = include_directories('../common')
common_util = files(
  '../common/Util.c',
  '../common/getopt.c',
)

time_engines = executable(
  'TimeEngines',
  ['TimeEngines.c', 'TimeUtil.c'] + common_util,
  include_directories : [common_inc],
  dependencies : [randompack_dep],
)

time_dist_c = executable(
  'TimeDistributions',
  ['TimeDistributions.c', 'TimeUtil.c'] + common_util,
  include_directories : [common_inc],
  dependencies : [randompack_dep],
)

time_norm_exp = executable(
  'TimeNormExp',
  ['TimeNormExp.c', 'TimeUtil.c'] + common_util,
  include_directories : [common_inc],
  dependencies : [randompack_dep],
)

time_logexp_src = ['TimeLogExp.c'] + common_util
time_logexp_c_args = []
if build_avx2
  time_logexp_src += ['../src/sleef.c']
  time_logexp_c_args = avx2_c_args
endif

time_log_exp = executable(
  'TimeLogExp',
  ['TimeUtil.c'] + time_logexp_src,
  include_directories : [common_inc],
  dependencies : [randompack_dep],
  c_args: time_logexp_c_args,
)

time_integers = executable(
  'TimeIntegers',
  ['TimeIntegers.c', 'TimeUtil.c'] + common_util,
  include_directories : [common_inc],
  dependencies : [randompack_dep],
)

if build_cpp
  time_dist_cpp = executable(
    'TimeDistCpp',
    'TimeDistCpp.cpp',
    dependencies : [randompack_dep],
  )
endif

if build_fortran
  darwin_link_args = host_machine.system() == 'darwin' ? ['-Wl,-no_warn_duplicate_libraries'] : []
  time_distributions_f = executable(
    'TimeDistFortran',
    'TimeDistFortran.f90',
    link_language: 'fortran',
    dependencies : [randompack_dep],
    link_args : darwin_link_args,
  )
endif
