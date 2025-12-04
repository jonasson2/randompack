test_sources = files(
  'RunTests.c',
  'TestRandomCreate.c',
  'TestRandomInt.c',
  'TestRandomNorm.c',
  'TestRandomPerm.c',
  'TestRandomRandomize.c',
  'TestRandomSample.c',
  'TestRandomU01.c',
  'TestRandomMvn.c',
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
