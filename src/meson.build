src_files = files(
  'randompack.c',
  'printX.c',
)

if host_machine.system() == 'darwin'
  src_files += files(
    'lapack_dpstf2.f',
    'lapack_dpstrf.f')
endif

librandompack = static_library(
  'randompack',
  src_files,
  include_directories: inc,
  dependencies: [blas_dep],
  pic: true,
  install: true,
)

randompack_dep = declare_dependency(
  link_with: librandompack,
  include_directories: inc,
  dependencies: [blas_dep],
)
