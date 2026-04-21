# Development note:
#
# To install in editable mode use:
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
  version: '0.1.2',
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
core_sources = files(
  'src/randompack.c',
  'src/randompack_float.c',
  'src/pcg_advance.c',
)

deps = [py_dep]

host_system = host_machine.system()
build_avx2 = false
avx2_c_args = []
build_avx512 = false
avx512_c_args = []
build_arm_sleef = (
  host_machine.cpu_family() in ['aarch64', 'arm64'] and
  host_machine.system() != 'darwin'
)

if host_machine.cpu_family() in ['x86', 'x86_64']
  if cc.get_id() in ['gcc', 'clang', 'appleclang', 'msvc', 'intel-llvm']
    build_avx2 = true
    build_avx512 = cc.get_id() != 'msvc'
  endif
endif

if build_avx2
  if cc.get_id() == 'msvc'
    avx2_c_args = ['/arch:AVX2']
  else
    avx2_c_args = cc.get_supported_arguments(['-mavx2', '-mfma'])
    build_avx2 = avx2_c_args.length() == 2
  endif
endif

if build_avx512
  avx512_c_args = cc.get_supported_arguments(['-mavx512f', '-mavx512dq'])
  build_avx512 = avx512_c_args.length() == 2
endif

arm_sleef_c_args = build_arm_sleef ? cc.get_supported_arguments(
  ['-Wno-attributes']
) : []

math_dep = cc.find_library('m', required: false)
if math_dep.found()
  deps += [math_dep]
endif

extra_sources = []
extra_objs = []
core_c_args = []
if cc.has_argument('-O3')
  core_c_args += ['-O3']
endif
if build_avx2
  core_c_args += ['-DBUILD_AVX2']
endif
if build_avx512
  core_c_args += ['-DBUILD_AVX512']
endif
core_c_args += arm_sleef_c_args

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
  elif blas_option == 'auto' or blas_option == 'system'
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

  if (blas_explicit or blas_option == 'system') and not blas_dep.found()
    error('Requested system BLAS not found: ' + blas_option)
  endif

  if not blas_dep.found() and not using_builtin_blas and blas_option != 'system'
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
  core_c_args += ['-DLOCAL_DPSTRF']
  extra_sources += files('src/rp_dpstrf.c')
  if using_builtin_blas
    add_languages('fortran', required: true, native: false)
    extra_sources += files('src/blas.f')
  endif
endif

if using_accelerate and (not using_builtin_blas)
  core_c_args += ['-DUSE_ACCEL_VV']
endif

if build_avx2
  avx2_lib = static_library(
    'randompack_avx2',
    files(
      'src/avx2.c',
      'src/inplace_logexp.c',
      'src/sleef.c',
    ),
    include_directories: inc,
    c_args: avx2_c_args + ['-DBUILD_AVX2'],
    pic: true,
    install: false,
  )
  extra_objs += [avx2_lib.extract_all_objects(recursive: false)]
endif

if build_arm_sleef
  arm_sleef_lib = static_library(
    'randompack_arm_sleef',
    files(
      'src/inplace_logexp.c',
      'src/sleef_arm.c',
    ),
    include_directories: inc,
    c_args: arm_sleef_c_args,
    pic: true,
    install: false,
  )
  extra_objs += [arm_sleef_lib.extract_all_objects(recursive: false)]
endif

if build_avx512
  avx512_lib = static_library(
    'randompack_avx512',
    files(
      'src/avx512.c',
      'src/sleef_avx512.c',
    ),
    include_directories: inc,
    c_args: avx512_c_args + ['-DBUILD_AVX512'],
    pic: true,
    install: false,
  )
  extra_objs += [avx512_lib.extract_all_objects(recursive: false)]
endif

py.extension_module(
  '_core',
  sources: wrap_sources + core_sources + extra_sources,
  objects: extra_objs,
  include_directories: inc,
  dependencies: deps,
  c_args: core_c_args,
  install: true,
  subdir: 'randompack',
)

py.install_sources('randompack/__init__.py', subdir: 'randompack')
