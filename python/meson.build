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
# Within the same Python environment, simply: >>> import randompack. The
# editable loader will trigger a rebuild if sources changed. Rerunning pip is
# only needed if pyproject.toml, package layout, or Python version changes.

project('randompack', 'c',
  version: '0.1.1',
  default_options: ['c_std=c11']
)

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

conda_option = get_option('conda')
blas_dep = disabler()
lapack_dep = disabler()
blas_found = 'none'
using_builtin_blas = false
using_accelerate = false

if conda_option
  blas_dep = dependency('blas', method: 'pkg-config', required: true)
  lapack_dep = dependency('lapack', method: 'pkg-config', required: true)
  blas_found = 'pkg-config'
  deps += [blas_dep, lapack_dep]
  message('Using BLAS/LAPACK from pkg-config')
else
  blas_option = get_option('blas')
  blas_backends = []
  blas_explicit = (blas_option != 'builtin' and blas_option != 'auto')

  if blas_option == 'builtin'
    using_builtin_blas = true
  elif blas_option == 'auto'
    blas_backends = ['accelerate', 'mkl', 'openblas']
  else
    blas_backends = [blas_option]
  endif

  foreach blas_name : blas_backends
    if blas_name == 'accelerate'
      if host_system == 'darwin'
        blas_dep = dependency('appleframeworks', modules: ['Accelerate'], required: false)
        if blas_dep.found()
          blas_found = 'Accelerate'
        endif
      endif
    elif blas_name == 'mkl'
      foreach mkl_variant : ['mkl-dynamic-lp64-iomp', 'mkl-static-lp64-iomp', 'mkl']
        blas_dep = dependency(mkl_variant, required: false)
        if blas_dep.found()
          break
        endif
      endforeach
      if not blas_dep.found()
        blas_dep = cc.find_library('mkl_rt', required: false)
      endif
      if blas_dep.found()
        blas_found = 'MKL'
      endif
    elif blas_name == 'openblas'
      blas_dep = dependency('openblas', required: false)
      if not blas_dep.found()
        blas_dep = cc.find_library('openblas', required: false)
      endif
      if blas_dep.found()
        blas_found = 'OpenBLAS'
      endif
    else
      error('Unknown blas option: ' + blas_name)
    endif

    if blas_dep.found()
      break
    endif
  endforeach

  if blas_explicit and not blas_dep.found()
    error('Requested system BLAS not found: ' + blas_option)
  endif

  if not blas_dep.found() and not using_builtin_blas
    using_builtin_blas = true
  endif

  if using_builtin_blas
    message('Using built-in BLAS from src/blas.f')
  else
    message('Using BLAS library: ' + blas_found)
    deps += [blas_dep]
  endif

  using_accelerate = (host_system == 'darwin') and (blas_found == 'Accelerate')
endif

local_dpstrf = using_accelerate or using_builtin_blas

if local_dpstrf
  add_languages('fortran', required: true, native: false)
  extra_sources += files('src/lapack_dpstrf.f')
  core_c_args += ['-DLOCAL_DPSTRF']
  if using_builtin_blas
    extra_sources += files('src/blas.f')
  endif
endif

if using_accelerate and (not using_builtin_blas)
  core_c_args += ['-DUSE_ACCEL_VV']
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
