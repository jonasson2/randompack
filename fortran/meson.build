fortran_src = []
fortran_inc = []

if build_fortran_interface
  py = import('python').find_installation()
  # randompack.f90 uses Fortran include files for the C bindings and method bodies.
  # Some toolchains do not track changes to those .inc files reliably, which can
  # leave stale Fortran objects in the build directory after API renames.
  randompack_f90 = custom_target(
    'randompack_combined_f90',
    input: [
      'src/randompack.f90',
      'src/randompack_c.inc',
      'src/util.inc',
      'src/methods.inc',
    ],
    output: 'randompack_combined.f90',
    command: [py, files('src/gen_randompack_f90.py'), '@OUTPUT@', '@INPUT@'],
  )
  fortran_src = [randompack_f90]
  fortran_inc = [include_directories('src')]
endif
