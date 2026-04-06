common_inc = include_directories('../common')
benchmark_sleef_warn_c_args = cc.get_supported_arguments(['-Wno-unused-function'])
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
time_logexp_dependencies = [randompack_dep]
if build_avx2
  time_logexp_src += ['../src/sleef.c']
  time_logexp_src += ['../src/inplace_logexp.c']
  time_logexp_src += ['sleef_logu10.c']
  time_logexp_c_args = avx2_c_args + benchmark_sleef_warn_c_args
endif
if host_machine.cpu_family() in ['aarch64', 'arm64'] and not force_nosimd
  time_logexp_src += ['../src/inplace_logexp.c']
  time_logexp_src += ['../src/sleef_arm.c']
  time_logexp_c_args += cc.get_supported_arguments(['-Wno-attributes'])
endif
if build_avx512
  sleef_avx512_lib = static_library(
    'benchmark_sleef_avx512',
    files('../src/sleef_avx512.c'),
    c_args: avx512_c_args + benchmark_sleef_warn_c_args,
    pic: true,
    install: false,
  )
  time_logexp_dependencies += declare_dependency(
    objects: [sleef_avx512_lib.extract_all_objects(recursive: false)],
  )
endif

time_log_exp = executable(
  'TimeLogExp',
  ['TimeUtil.c'] + time_logexp_src,
  include_directories : [common_inc],
  dependencies : time_logexp_dependencies,
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
