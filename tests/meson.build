test_sources = files(
  'RunTests.c',
  'TestCreate.c',
  'TestInt.c',
  'TestPerm.c',
  'TestSample.c',
  'TestNormal.c',
  'TestExp.c',
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
)

run_tests = executable(
  'RunTests',
  test_sources + extra_util,
  dependencies : [randompack_dep],
)

test('RunTests', run_tests, args : ['-v'])

seedseq_test = executable(
  'TestSeedSeq',
  ['TestSeedSeq.cpp'],
  include_directories : [inc],
)

test('TestSeedSeq', seedseq_test)
