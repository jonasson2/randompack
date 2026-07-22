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
  benchmark_cpp_fast_args = cpp.get_supported_arguments(['-U_GLIBCXX_ASSERTIONS'])
  time_dist_cpp = executable(
    'TimeDistCpp',
    'TimeDistCpp.cpp',
    dependencies : [randompack_dep],
    cpp_args : benchmark_cpp_fast_args,
  )

  time_int_cpp = executable(
    'TimeIntCpp',
    'TimeIntCpp.cpp',
    dependencies : [randompack_dep],
    cpp_args : benchmark_cpp_fast_args,
  )
endif

mkl_bench_dep = disabler()
mkl_bench_found = false
mkl_include_args = []
mkl_root_candidates = []

foreach mkl_pkg : ['mkl-dynamic-lp64-seq', 'mkl-dynamic-lp64-gomp', 'mkl-dynamic-lp64-iomp', 'mkl-sdl']
  mkl_bench_dep = dependency(mkl_pkg, required : false)
  if mkl_bench_dep.found()
    mkl_bench_found = true
    message('Using MKL benchmark dependency via pkg-config: ' + mkl_pkg)
    break
  endif
endforeach

mkl_root_env = run_command('sh', '-c', 'printf %s "$MKLROOT"', check : true).stdout().strip()
if mkl_root_env != ''
  mkl_root_candidates += [mkl_root_env]
endif
mkl_root_candidates += ['/opt/intel/oneapi/mkl/latest']

if not mkl_bench_found
  foreach mkl_root : mkl_root_candidates
    mkl_inc_dir = join_paths(mkl_root, 'include')
    if cc.has_header('mkl.h', args : ['-I' + mkl_inc_dir])
      mkl_lib_dirs = [join_paths(mkl_root, 'lib', 'intel64'), join_paths(mkl_root, 'lib')]
      mkl_bench_dep = cc.find_library('mkl_rt', dirs : mkl_lib_dirs, required : false)
      if mkl_bench_dep.found()
        mkl_include_args = ['-I' + mkl_inc_dir]
        mkl_bench_found = true
        message('Using MKL benchmark library from ' + mkl_root)
        break
      endif
    endif
  endforeach
endif

if mkl_bench_found
  time_dist_mkl = executable(
    'TimeDistMKL',
    'TimeDistMKL.c',
    dependencies : [randompack_dep, mkl_bench_dep],
    c_args : mkl_include_args,
  )

  time_int_mkl = executable(
    'TimeIntMKL',
    'TimeIntMKL.c',
    dependencies : [randompack_dep, mkl_bench_dep],
    c_args : mkl_include_args,
  )
else
  message('Skipping TimeDistMKL and TimeIntMKL benchmarks (MKL headers or libraries not found)')
endif

if build_fortran_interface
  darwin_link_args = host_machine.system() == 'darwin' ? ['-Wl,-no_warn_duplicate_libraries'] : []
  time_distributions_f = executable(
    'TimeDistFortran',
    '../fortran/examples/TimeDistFortran.f90',
    link_language: 'fortran',
    dependencies : [randompack_fortran_dep],
    link_args : darwin_link_args,
  )

  time_dist_gfortran = executable(
    'TimeDistGfortran',
    'TimeDistGfortran.f90',
    link_language: 'fortran',
    dependencies : [randompack_fortran_dep],
    link_args : darwin_link_args,
  )
endif
