#ifndef TESTS_H
#define TESTS_H

#include <stdbool.h>

void TestCreate(void);
void TestRandomize(void);
void TestInt(void);
void TestLongLong(void);
void TestPerm(void);
void TestSample(void);
void TestU01(void);
void TestUnif(void);
void TestMvn(void);
void TestUint8(void);
void TestUint16(void);
void TestUint32(void);
void TestUint64(void);
void TestNorm(void);
void TestContinuous(void);
void TestHelpers(void);
void TestOpenlibm(void);
void TestFullMantissa(void);
void TestLogExp(void);
void TestSleefMath(void);
void TestAvx2(void);
void TestSeed(void);
void TestSetState(void);
void TestReference(void);
void TestBuffer(void);
#endif /* TESTS_H */
