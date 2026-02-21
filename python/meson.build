# Development note:
#
# To installed in editable mode use:
#
#   pip install -e . --no-build-isolation
#
# This command (via meson-python) automatically runs:
#   meson setup <build-dir>
#   meson compile
#
# where <build-dir> is in the active Python environment’s site-packages folder
# (outside randompack's folder tree).
#
#
# Within the same Python environment, simply: >>> import randompack. The
# editable loader will trigger a rebuild if sources changed. Rerunning pip is
# only needed if pyproject.toml, package layout, or Python version changes.

project('randompack', 'c',
  version: '0.1.0',
  default_options: ['c_std=c11'])

py = import('python').find_installation(pure: false)
cc = meson.get_compiler('c')
py_dep = py.dependency()

numpy_inc = run_command(py, ['-c', 'import numpy; print(numpy.get_include())'],
  check: true).stdout().strip()
inc = include_directories('src', numpy_inc)

cython = find_program('cython', required: true)
wrap_sources = [custom_target(
  'cython_core',
  input: 'randompack/_core.pyx',
  output: 'randompack__core.c',
  command: [cython, '-3', '@INPUT@', '-o', '@OUTPUT@'],
)]
core_sources = files('src/randompack.c')

deps = [py_dep]

host_system = host_machine.system()

math_dep = cc.find_library('m', required: false)
if math_dep.found()
  deps += [math_dep]
endif

extra_sources = []
core_c_args = []

if host_system == 'darwin'
  # Use Apple Accelerate (BLAS + LAPACK). Do NOT compile the fallback Fortran.
  accel_dep = dependency('appleframeworks', modules: ['Accelerate'], required: true)
  deps += [accel_dep]
  add_languages('fortran', required: true)
  extra_sources += files('src/lapack_dpstrf.f')
  core_c_args += ['-DLOCAL_DPSTRF']
else
  # Windows/Linux: build and link the bundled BLAS/LAPACK subset.
  add_languages('fortran', required: true)
  extra_sources += files('src/blas.f', 'src/lapack_dpstrf.f')
  core_c_args += ['-DLOCAL_DPSTRF']
endif

py.extension_module(
  '_core',
  sources: wrap_sources + core_sources + extra_sources,
  include_directories: inc,
  dependencies: deps,
  c_args: core_c_args,
  install: true,
  subdir: 'randompack',
)

py.install_sources('randompack/__init__.py', subdir: 'randompack')
