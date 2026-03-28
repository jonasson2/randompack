common_inc = include_directories('../common')
common_util = files(
  '../common/Util.c',
  '../common/getopt.c',
)

threads_dep = dependency('threads', required: false)
pthreads_ok = false
if threads_dep.found()
  cc = meson.get_compiler('c')
  pthreads_ok = cc.has_header('pthread.h')
endif
mpi_dep = dependency('mpi', required: false)

run_varmasim = executable(
  'RunRandom',
  ['RunRandom.c'] + common_util,
  include_directories : [common_inc],
  dependencies : [randompack_dep],
)

run_exp = executable(
  'RunExp',
  ['RunExp.c'] + common_util,
  include_directories : [common_inc],
  dependencies : [randompack_dep],
)

time_engines = executable(
  'TimeEngines',
  ['TimeEngines.c', 'TimeUtil.c'] + common_util,
  include_directories : [common_inc],
  dependencies : [randompack_dep],
)

time_fast = executable(
  'TimeFast',
  ['TimeFast.c', 'TimeUtil.c'] + common_util,
  include_directories : [common_inc],
  dependencies : [randompack_dep],
)

time_fast_direct = executable(
  'TimeFastDirect',
  ['TimeFastDirect.c', 'TimeUtil.c'] + common_util,
  include_directories : [common_inc],
  dependencies : [randompack_dep],
)

time_distributions = executable(
  'TimeDistC',
  ['TimeDistC.c', 'TimeUtil.c'] + common_util,
  include_directories : [common_inc],
  dependencies : [randompack_dep],
)

time_norm = executable(
  'TimeNormC',
  ['TimeNormC.c', 'TimeUtil.c'] + common_util,
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

time_logarithms = executable(
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

examine_resolution = executable(
  'ExamineResolution',
  'ExamineResolution.c',
  include_directories : [common_inc],
  dependencies : [randompack_dep],
)

draw_example = executable(
  'DrawDemo',
  'DrawDemo.c',
  include_directories : [common_inc],
  dependencies : [randompack_dep],
)

raw_stream = executable(
  'RawStream',
  ['RawStream.c'] + common_util,
  include_directories : [common_inc],
  dependencies : [randompack_dep],
)

if build_fortran
  darwin_link_args = host_machine.system() == 'darwin' ? ['-Wl,-no_warn_duplicate_libraries'] : []
  check_fortran_ints = executable(
    'CheckFortranInts',
    'CheckFortranInts.f90',
    link_language: 'fortran',
    dependencies : [randompack_dep],
    link_args : darwin_link_args,
  )
  time_distributions_f = executable(
    'TimeDistFortran',
    'TimeDistFortran.f90',
    link_language: 'fortran',
    dependencies : [randompack_dep],
    link_args : darwin_link_args,
  )
endif

testu01_prefix = get_option('TestU01')
if testu01_prefix != ''
  testu01_inc = include_directories(join_paths(testu01_prefix, 'include'))
  testu01_libdir = join_paths(testu01_prefix, 'lib')
  testu01_libs = [
    cc.find_library('testu01', dirs: testu01_libdir, required: true,
      static: true),
    cc.find_library('probdist', dirs: testu01_libdir, required: true,
      static: true),
    cc.find_library('mylib', dirs: testu01_libdir, required: true,
      static: true),
  ]
  testu01_driver = executable(
    'TestU01Driver',
    'TestU01Driver.c',
    dependencies : [randompack_dep] + testu01_libs,
    include_directories : [testu01_inc, common_inc],
  )
  testu01_pit = executable(
    'TestU01PIT',
    'TestU01PIT.c',
    dependencies : [randompack_dep] + testu01_libs,
    include_directories : [testu01_inc, common_inc],
  )
  test_sample_prod = executable(
    'TestSampleProd',
    'TestSampleProd.c',
    dependencies : [randompack_dep] + testu01_libs,
    include_directories : [testu01_inc, common_inc],
  )
endif

if threads_dep.found() and pthreads_ok
  thread_example = executable(
    'ThreadExample',
    'ThreadExample.c',
    include_directories : [common_inc],
    dependencies : [randompack_dep, threads_dep],
  )
endif

if mpi_dep.found() and threads_dep.found() and pthreads_ok
  mpi_thread_example = executable(
    'MpiThreadExample',
    'MpiThreadExample.c',
    include_directories : [common_inc],
    dependencies: [
      randompack_dep,
      threads_dep,
      mpi_dep,
    ],
  )
endif
