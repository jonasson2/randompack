randompack_src = files(
  'randompack.c',
  'printX.c',
)

randompack_src += fortran_src

u01_hires = get_option('u01_hires')
u01_hires_args = u01_hires ? ['-DU01_HIRES'] : []

if accel_bug_workaround
  randompack_src += files(
    'lapack_dpstf2.f',
    'lapack_dpstrf.f')
endif

randompack_inc = [inc] + fortran_inc

librandompack_static = static_library(
  'randompack',
  randompack_src,
  include_directories: randompack_inc,
  dependencies: [blas_dep] + math_deps,
  c_args: u01_hires_args + accel_bug_args,
  pic: true,
  install: true,
)

librandompack = shared_library(
  'randompack',
  randompack_src,
  include_directories: randompack_inc,
  dependencies: [blas_dep] + math_deps,
  c_args: u01_hires_args + accel_bug_args,
  install: true,
)

install_headers(
  'randompack.h',
  subdir: 'randompack',
)

randompack_dep = declare_dependency(
  link_with: librandompack,
  include_directories: randompack_inc,
  dependencies: [blas_dep] + math_deps,
  compile_args: u01_hires_args + accel_bug_args,
)

if get_option('build_test_variants')
  librandompack_no128 = static_library(
    # Pretend HAVE128 is disabled
    'randompack_no128',
    randompack_src,
    include_directories: randompack_inc,
    dependencies: [blas_dep] + math_deps,
    c_args: ['-DRANDOMPACK_PRETEND_NO128'] + accel_bug_args,
    pic: true,
    install: false,
  )

  randompack_no128_dep = declare_dependency(
    link_with: librandompack_no128,
    include_directories: randompack_inc,
    dependencies: [blas_dep] + math_deps,
    compile_args: accel_bug_args,
  )
endif
