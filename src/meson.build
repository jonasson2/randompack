randompack_src = files(
  'randompack.c',
  'printX.c',
)

if host_machine.system() == 'darwin'
  randompack_src += files(
    'lapack_dpstf2.f',
    'lapack_dpstrf.f')
endif

librandompack = static_library(
  'randompack',
  randompack_src,
  include_directories: inc,
  dependencies: [blas_dep] + math_deps,
  pic: true,
  install: true,
)

librandompack_no128 = static_library(
  'randompack_no128',
  randompack_src,
  include_directories: inc,
  dependencies: [blas_dep] + math_deps,
  c_args: ['-DRANDOMPACK_NO_INT128'],
  pic: true,
  install: false,
)

install_headers(
  'randompack.h',
  subdir: 'randompack',
)

randompack_dep = declare_dependency(
  link_with: librandompack,
  include_directories: inc,
  dependencies: [blas_dep] + math_deps,
)

randompack_no128_dep = declare_dependency(
  link_with: librandompack_no128,
  include_directories: inc,
  dependencies: [blas_dep] + math_deps,
)
