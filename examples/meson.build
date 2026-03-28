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

run_random = executable(
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
