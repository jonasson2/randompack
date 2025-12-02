src_files = files(
  'randompack.c',
  'printX.c',
  'lapack_dpstf2.f',
  'lapack_dpstrf.f',
)

librandompack = library(
  'randompack',
  src_files,
  include_directories : inc,
  dependencies : [blas_dep],
)

randompack_dep = declare_dependency(
  link_with : librandompack,
  include_directories : inc,
  dependencies : [blas_dep],
)
