test_sources = files(
  'RunTests.c',
  'TestCreate.c',
  'TestInt.c',
  'TestNorm.c',
  'TestPerm.c',
  'TestRandomize.c',
  'TestSample.c',
  'TestU01.c',
  'TestMvn.c',
  'TestUint32.c',
)

extra_util = files(
  'ExtraUtil.c',
  'xCheck.c',
)

run_tests = executable(
  'RunTests',
  test_sources + extra_util,
  dependencies : [randompack_dep],
)

test('RunTests', run_tests, args : ['-v'])
