project('randompack', 'c',
  version: '0.1.0',
  default_options: ['c_std=c11'])

py = import('python').find_installation(pure: false)
cc = meson.get_compiler('c')

py_dep = py.dependency()

numpy_inc = run_command(py, ['-c', 'import numpy; print(numpy.get_include())'],
                        check: true).stdout().strip()

inc = include_directories('src', numpy_inc)

wrap_sources = files('randompack/_core.c')
core_sources = files('src/randompack.c')

math_dep = cc.find_library('m', required: false)

lapack_dep = dependency('lapack', required: false)
if not lapack_dep.found()
  lapack_dep = cc.find_library('lapack', required: false)
endif
if not lapack_dep.found()
  error('LAPACK not found (needed for MVN / dpotrf_)')
endif

blas_dep = dependency('blas', required: false)
if not blas_dep.found()
  blas_dep = cc.find_library('blas', required: false)
endif
if not blas_dep.found()
  error('BLAS not found (needed for MVN)')
endif

deps = [py_dep, lapack_dep, blas_dep]
if math_dep.found()
  deps += [math_dep]
endif

py.extension_module(
  '_core',
  sources: wrap_sources + core_sources,
  include_directories: inc,
  dependencies: deps,
  install: true,
  subdir: 'randompack',
)

py.install_sources('randompack/__init__.py', subdir: 'randompack')
