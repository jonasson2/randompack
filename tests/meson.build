add_languages('cpp', required: true, native: false)

common_inc = include_directories('../common')
common_util = files(
  '../common/Util.c',
  '../common/getopt.c',
)

test_sources = files(
  'RunTests.c',
  'TestCreate.c',
  'TestInt.c',
  'TestLongLong.c',
  'TestPerm.c',
  'TestSample.c',
  'TestExp.c',
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
  'TestX256ppsimd.c',
  'TestRanluxpp.c',
  'TestDpstrf.c',
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

blas_test_args = using_builtin_blas ? ['-DRANDOMPACK_BUILTIN_BLAS'] : []

run_tests = executable(
  'RunTests',
  test_sources + extra_util,
  include_directories: [common_inc],
  dependencies : [randompack_dep],
  c_args : ['-DRANDOMPACK_TEST_HOOKS'] + blas_test_args,
)

test('RunTests', run_tests, args : ['-v'])

run_tests_buf8 = executable(
  # Small buffer test to stress the buffer draw machinery
  'RunTestsBuf8',
  test_sources + extra_util,
  include_directories: [common_inc],
  dependencies : [randompack_buf8_dep],
  c_args : ['-DRANDOMPACK_TEST_HOOKS', '-DBUFSIZE=8'] + blas_test_args,
)

test('RunTestsBuf8', run_tests_buf8, args : ['-v'])

if build_fortran_interface
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

seedseq_test = executable(
  'TestSeedSeq',
  ['TestSeedSeq.cpp'],
  include_directories : [inc, common_inc],
)

test('TestSeedSeq', seedseq_test)
