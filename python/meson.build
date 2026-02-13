project('randompack', 'c',
  version: '0.1.0',
  default_options: ['c_std=c11'])

py = import('python').find_installation(pure: false)

py_inc = run_command(py, ['-c', 'import sysconfig; print(sysconfig.get_path("include"))'], check: true).stdout().strip()
numpy_inc = run_command(py, ['-c', 'import numpy; print(numpy.get_include())'], check: true).stdout().strip()

inc = include_directories('src', py_inc, numpy_inc)

wrap_sources = files('randompack/_core.c')

core_sources = files(
  'src/randompack.c'
)

py.extension_module(
  '_core',
  sources: wrap_sources + core_sources,
  include_directories: inc,
  install: true,
  subdir: 'randompack',
)
py.install_sources('randompack/__init__.py', subdir: 'randompack')
