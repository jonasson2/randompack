test_sources = files(
  'RunTests.c',
  'TestCreate.c',
  'TestInt.c',
  'TestPerm.c',
  'TestSample.c',
  'TestNormal.c',
  'TestU01.c',
  'TestMvn.c',
  'TestUint32.c',
  'TestUint64.c',
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
