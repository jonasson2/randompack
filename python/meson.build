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

deps = [py_dep]

host_system = host_machine.system()

math_dep = cc.find_library('m', required: false)
if math_dep.found()
  deps += [math_dep]
endif

extra_sources = []

if host_system == 'darwin'
  # Use Apple Accelerate (BLAS + LAPACK). Do NOT compile the fallback Fortran.
  accel_dep = dependency('appleframeworks', modules: ['Accelerate'], required: true)
  deps += [accel_dep]
  add_languages('fortran', required: true)
  extra_sources += files('src/lapack_dpstf2.f', 'src/lapack_dpstrf.f')
else
  # Windows/Linux: build and link the bundled BLAS/LAPACK subset.
  add_languages('fortran', required: true)
  extra_sources += files('src/blas.f')
endif

py.extension_module(
  '_core',
  sources: wrap_sources + core_sources + extra_sources,
  include_directories: inc,
  dependencies: deps,
  install: true,
  subdir: 'randompack',
)

py.install_sources('randompack/__init__.py', subdir: 'randompack')
