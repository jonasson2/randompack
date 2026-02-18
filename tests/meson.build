add_languages('cpp', required: true)

test_sources = files(
  'RunTests.c',
  'TestCreate.c',
  'TestInt.c',
  'TestLongLong.c',
  'TestPerm.c',
  'TestSample.c',
  'TestNorm.c',
  'TestUnif.c',
  'TestContinuous.c',
  'TestU01.c',
  'TestMvn.c',
  'TestUint8.c',
  'TestUint16.c',
  'TestUint32.c',
  'TestUint64.c',
  'TestReference.c',
  'TestSeed.c',
  'TestSetState.c',
  'TestHelpers.c',
  'TestFullMantissa.c',
  'TestBitexact.c',
  'TestAvx2.c',
  'TestOpenlibm.c',
  'TestBuffer.c',
)

extra_util = files(
  'xCheck.c',
  'test_util.c',
  'test_cdfs.c',
  'incbet.c',
  'igam.c',
)

run_tests = executable(
  'RunTests',
  test_sources + extra_util,
  dependencies : [randompack_dep],
  c_args : ['-DRANDOMPACK_TEST_HOOKS'],
)

test('RunTests', run_tests, args : ['-v'])

if build_fortran
  darwin_link_args = host_machine.system() == 'darwin' ? ['-Wl,-no_warn_duplicate_libraries'] : []
  fortran_tests = executable(
    'RunFortranTests',
    '../fortran/tests/run_fortran_tests.f90',
    link_language: 'fortran',
    dependencies: [randompack_dep],
    link_args : darwin_link_args,
  )
  test('RunFortranTests', fortran_tests)
endif

if get_option('build_test_variants')
  test_no128 = executable(
    'RunTestNo128',
    [
      'RunTestNo128.c',
      'TestCreate.c',
      'TestHelpers.c',
    ] + extra_util,
    dependencies : [randompack_no128_dep],
    c_args : ['-DRANDOMPACK_PRETEND_NO128'],
  )

  test_nosimd = executable(
    'RunTestNoSimd',
    test_sources + extra_util,
    dependencies : [randompack_nosimd_dep],
  )

  test_variants = find_program('sh')
  test('TestVariants', test_variants,
    args : [files('RunTestVariants.sh'), meson.current_build_dir()],
    depends : [test_no128, test_nosimd])

endif

seedseq_test = executable(
  'TestSeedSeq',
  ['TestSeedSeq.cpp'],
  include_directories : [inc],
)

test('TestSeedSeq', seedseq_test)
