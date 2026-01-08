test_sources = files(
  'RunTests.c',
  'TestCreate.c',
  'TestInt.c',
  'TestPerm.c',
  'TestSample.c',
  'TestNorm.c',
  'TestNormal.c',
  'TestLognormal.c',
  'TestGumbel.c',
  'TestPareto.c',
  'TestExp.c',
  'TestGamma.c',
  'TestChi2.c',
  'TestBeta.c',
  'TestUnif.c',
  'TestT.c',
  'TestF.c',
  'TestWeibull.c',
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
  'TestBuffer.c',
)

extra_util = files(
  'xCheck.c',
  'TestUtil.c',
  'incbet.c',
  'igam.c',
)

run_tests = executable(
  'RunTests',
  test_sources + extra_util,
  dependencies : [randompack_dep],
)

test('RunTests', run_tests, args : ['-v'])

# test_no128 = executable(
#   'randompack_tests_no128',
#   ['TestNo128.c'],
#   include_directories : [inc],
#   dependencies : [randompack_no128_dep],
#   c_args : ['-DRANDOMPACK_NO_INT128'],
# )
#
# test('randompack_tests_no128', test_no128)

seedseq_test = executable(
  'TestSeedSeq',
  ['TestSeedSeq.cpp'],
  include_directories : [inc],
)

test('TestSeedSeq', seedseq_test)
