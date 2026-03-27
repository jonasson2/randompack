#ifndef TESTS_H
#define TESTS_H

#include <stdbool.h>

void TestCreate(void);
void TestRandomize(void);
void TestInt(void);
void TestIntx(char *engine);
void TestLongLong(void);
void TestPerm(void);
void TestSample(void);
void TestExp(void);
void TestExpx(char *engine);
void TestUnif(void);
void TestUnifx(char *engine);
void TestMvn(void);
void TestUint8(void);
void TestUint16(void);
void TestUint32(void);
void TestUint32x(char *engine);
void TestUint64(void);
void TestUint64x(char *engine);
void TestNorm(void);
void TestNormx(char *engine);
void TestContinuous(void);
void TestContinuousx(char *engine);
void TestHelpers(void);
void TestOpenlibm(void);
void TestFullMantissa(void);
void TestBitexact(void);
void TestBitexactx(char *engine);
void TestAvx2(void);
void TestSeed(void);
void TestSetState(void);
void TestReference(void);
void TestX256ppsimd(void);
void TestSfc64simd(void);
void TestRanluxpp(void);
void TestDpstrf(void);
void TestBuffer(void);
void TestBufferx(char *engine);
void TestReferencex(char *engine);

#endif /* TESTS_H */
