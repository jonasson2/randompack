example_util = files(
  'getopt.c',
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
  ['RunRandom.c'] + example_util,
  dependencies : [randompack_dep],
)

time_engines = executable(
  'TimeEngines',
  ['TimeEngines.c', 'TimeUtil.c'] + common_util,
  dependencies : [randompack_dep],
)

time_distributions = executable(
  'TimeDistributions',
  ['TimeDistributions.c', 'TimeUtil.c'] + common_util,
  dependencies : [randompack_dep],
)

time_integers = executable(
  'TimeIntegers',
  ['TimeIntegers.c'] + common_util,
  dependencies : [randompack_dep],
)

examine_resolution = executable(
  'ExamineResolution',
  'ExamineResolution.c',
  dependencies : [randompack_dep],
)

draw_example = executable(
  'draw',
  'draw.c',
  dependencies : [randompack_dep],
)

if threads_dep.found() and pthreads_ok
  thread_example = executable(
    'ThreadExample',
    'ThreadExample.c',
    dependencies : [randompack_dep, threads_dep],
  )
endif

if mpi_dep.found() and threads_dep.found() and pthreads_ok
  mpi_thread_example = executable(
    'MpiThreadExample',
    'MpiThreadExample.c',
    dependencies: [
      randompack_dep,
      threads_dep,
      mpi_dep,
    ],
  )
endif
