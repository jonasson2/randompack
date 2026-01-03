// Various utilities for the test functions
#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include "Util.h"
#include "randompack.h"
#include "randompack_config.h"
#include "cephes.h"

#define CHECK_VEC_EQUAL(TYPE, a, b, n) do {        \
  bool same = true;                                \
  for (int i = 0; i < (n); i++) {                  \
    if ((a)[i] != (b)[i]) {                        \
      same = false;                                \
      break;                                       \
    }                                              \
  }                                                \
  xCheck(same);                                    \
 } while (0)

#define CHECK_VEC_DIFFERENT(TYPE, a, b, n) do { \
  bool diff = false;                            \
  for (int i = 0; i < (n); i++) {               \
    if ((a)[i] != (b)[i]) {                     \
      diff = true;                              \
      break;                                    \
    }                                           \
  }                                             \
  xCheck(diff);                                 \
 } while (0)

#define DRAW(engine, seed, CALL) do {									\
	randompack_rng *rng = create_seeded_rng((engine), (seed));	\
	check_rng_clean(rng);													\
	bool ok = (CALL);															\
	check_success(ok, rng);													\
	randompack_free(rng);													\
 } while (0)

// Check same seed => same draws, different seeds => different draws
#define TEST_DETERMINISM0(engine, TYPE, FUNC) do {		\
	TYPE x[3], y[3], z[3];										\
	int n = LEN(x);												\
	DRAW((engine), 42, randompack_##FUNC(x, n, rng));	\
	DRAW((engine), 42, randompack_##FUNC(y, n, rng));	\
	DRAW((engine), 43, randompack_##FUNC(z, n, rng));	\
	CHECK_VEC_EQUAL(TYPE, x, y, n);							\
	CHECK_VEC_DIFFERENT(TYPE, x, z, n);						\
 } while (0)

#define TEST_DETERMINISM1(engine, TYPE, FUNC, p1) do {		\
	TYPE x[3], y[3], z[3];												\
	int n = LEN(x);														\
	DRAW((engine), 42, randompack_##FUNC(x, n, (p1), rng));	\
	DRAW((engine), 42, randompack_##FUNC(y, n, (p1), rng));	\
	DRAW((engine), 43, randompack_##FUNC(z, n, (p1), rng));	\
	CHECK_VEC_EQUAL(TYPE, x, y, n);									\
	CHECK_VEC_DIFFERENT(TYPE, x, z, n);								\
 } while (0)

#define TEST_DETERMINISM2(engine, TYPE, FUNC, p1, p2) do {			\
	TYPE x[3], y[3], z[3];														\
	int n = LEN(x);																\
	DRAW((engine), 42, randompack_##FUNC(x, n, (p1), (p2), rng));	\
	DRAW((engine), 42, randompack_##FUNC(y, n, (p1), (p2), rng));	\
	DRAW((engine), 43, randompack_##FUNC(z, n, (p1), (p2), rng));	\
	CHECK_VEC_EQUAL(TYPE, x, y, n);											\
	CHECK_VEC_DIFFERENT(TYPE, x, z, n);										\
 } while (0)

// Check that zero-length buffers work ok, and that null buff and null rng fail
#define TEST_EDGE_CASES0(engine, TYPE, FUNC) do {							\
	TYPE buf[3] = {1, 2, 3};														\
	TYPE orig[3] = {1, 2, 3};														\
	bool ok;																				\
	randompack_rng *rng = create_seeded_rng((engine), 123);				\
	ok = randompack_##FUNC(buf, 0, rng); check_success(ok, rng);		\
	CHECK_VEC_EQUAL(TYPE, buf, orig, LEN(buf));								\
	ok = randompack_##FUNC(0, LEN(buf), rng); check_failure(ok, rng);	\
	ok = randompack_##FUNC(buf, LEN(buf), 0); xCheck(!ok);				\
	randompack_free(rng);															\
 } while (0)

#define TEST_EDGE_CASES1(engine, TYPE, FUNC, p1) do {								\
	TYPE buf[3] = {1, 2, 3};																\
	TYPE orig[3] = {1, 2, 3};																\
	bool ok;																						\
	randompack_rng *rng = create_seeded_rng((engine), 123);						\
	ok = randompack_##FUNC(buf, 0, (p1), rng); check_success(ok, rng);		\
	CHECK_VEC_EQUAL(TYPE, buf, orig, LEN(buf));										\
	ok = randompack_##FUNC(0, LEN(buf), (p1), rng); check_failure(ok, rng);	\
	ok = randompack_##FUNC(buf, LEN(buf), (p1), 0); xCheck(!ok);				\
	randompack_free(rng);																	\
 } while (0)

#define TEST_EDGE_CASES2(engine, TYPE, FUNC, p1, p2) do {							  \
	TYPE buf[4] = {1, 2, 3};																	  \
	TYPE orig[4] = {1, 2, 3};																	  \
	bool ok;																							  \
	randompack_rng *rng = create_seeded_rng((engine), 123);							  \
	ok = randompack_##FUNC(buf, 0, (p1), (p2), rng); check_success(ok, rng);	  \
	CHECK_VEC_EQUAL(TYPE, buf, orig, LEN(buf));											  \
	ok = randompack_##FUNC(0, LEN(buf), (p1), (p2), rng); check_failure(ok, rng);	\
	ok = randompack_##FUNC(buf, LEN(buf), (p1), (p2), 0); xCheck(!ok);			  \
	randompack_free(rng);																		  \
 } while (0)

#define TEST_ILLEGAL_PARAMS1(TYPE, engine, FUNC, bad1) do {	\
	TYPE buf[4];															\
	randompack_rng *rng = create_seeded_rng((engine), 123);	\
	bool ok = randompack_##FUNC(buf, LEN(buf), (bad1), rng);	\
	check_failure(ok, rng);												\
	randompack_free(rng);												\
 } while (0)

#define TEST_ILLEGAL_PARAMS2(TYPE, engine, FUNC, bad1, bad2) do {		\
	TYPE buf[4];																		\
	randompack_rng *rng = create_seeded_rng((engine), 123);				\
	bool ok = randompack_##FUNC(buf, LEN(buf), (bad1), (bad2), rng);  \
	check_failure(ok, rng);															\
	randompack_free(rng);															\
 } while (0)

#define TEST_SUPPORT(TYPE, x, n, a, b) do {		\
	TYPE xmin = (x)[0];									\
	TYPE xmax = (x)[0];									\
	for (int i=1; i<(n); i++) {						\
	  TYPE xi = (x)[i];									\
	  if (xi < xmin) xmin = xi;						\
	  if (xi > xmax) xmax = xi;						\
	}															\
	xCheck(xmin >= (a) && xmax <= (b));				\
 } while (0)

//------------------------------------------------------------------------------
// Engine name tables used by tests
//------------------------------------------------------------------------------
static char *engines[] = {
  "xorshift128+",
  "squares64",
  "xoshiro256**",
  "xoshiro256++",
  "chacha20",
  "philox"
#ifdef HAVE128
  , "pcg64_dxsm"
  , "cwg128_64"
#endif
};

static char *abbrev[] = {
  "x128+",
  "squares64",
  "x256**",
  "x256++",
  "chacha20",
  "philox"
#ifdef HAVE128
  , "pcg64"
  , "cwg128"
#endif
};

typedef struct {
  const char *name;
  int state_words;
} engine_table_entry;

static engine_table_entry engine_table[] = {
  {"xorshift128+",  2},
  {"squares64",     2},
  {"xoshiro256**",  4},
  {"xoshiro256++",  4},
  {"chacha20",      6},
  {"philox",        6},
#ifdef HAVE128
  {"pcg64_dxsm",    4},
  {"cwg128_64",     5},
#endif
};
//------------------------------------------------------------------------------
// Global test sizes / sample counts
//------------------------------------------------------------------------------

enum {
  N_BAL_CNTS = 5000000,
  N_BAL_BITS = 40000,
  N_STAT_FAST = 100000,
  N_STAT_SLOW = 20000
};
static const double TEST_P_VALUE = 1e-12;

//------------------------------------------------------------------------------
// Vector equality / difference helpers
//------------------------------------------------------------------------------

bool equal_vec(int *a, int *b, int n);                      // a = b?
bool equal_vecd(double *a, double *b, int n);               // a = b?
bool equal_vec8(uint8_t *a, uint8_t *b, int n);             // a = b?
bool equal_vec16(uint16_t *a, uint16_t *b, int n);          // a = b?
bool equal_vec32(uint32_t *a, uint32_t *b, int n);          // a = b?
bool equal_vec64(uint64_t *a, uint64_t *b, int n);          // a = b?
bool everywhere_different(uint64_t *a, uint64_t *b, int n); // ai ≠ bi for all i

//------------------------------------------------------------------------------
// Approximate equality
//------------------------------------------------------------------------------

int almostSame(double a, double b);
int almostEqual(double a[], double b[], int n);
int almostAllSame(double a[], int n);
int almostZero(double a[], int n);

//------------------------------------------------------------------------------
// Simple statistics
//------------------------------------------------------------------------------

double mean(double *x, int n);
double var(double *x, int n, double mu);
double skewness(double *x, int n, double xbar, double s2);
double kurtosis(double *x, int n, double xbar, double s2);
void cov(char *transp, int m, int n, double X[], double C[]);
bool check_meanvar(double *x, int n);
bool check_skewkurt(double *x, int n, double skew, double kurt);
bool check_u01_meanvar(double *x, int n);
bool check_u01_skewkurt(double *x, int n);
double normcdf(double x);
double normccdf(double x);
double probit(double p);
double chi2_cdf(double x, int nu);
double chi2_ccdf(double x, int nu);
double gamma_cdf(double x, double shape, double scale);
double gamma_cdfc(double x, double shape, double scale);

//------------------------------------------------------------------------------
// Min/max helpers for scalars and vectors
//------------------------------------------------------------------------------

uint8_t max8(uint8_t a, uint8_t b);
uint16_t max16(uint16_t a, uint16_t b);
uint32_t max32(uint32_t a, uint32_t b);
uint64_t max64(uint64_t a, uint64_t b);

int maxv(int *x, int n);
uint8_t maxv8(uint8_t *x, int n);
uint16_t maxv16(uint16_t *x, int n);
uint32_t maxv32(uint32_t *x, int n);
uint64_t maxv64(uint64_t *x, int n);
double maxvd(double *x, int n);

int minv(int *x, int n);
double minvd(double *x, int n);

//------------------------------------------------------------------------------
// RNG / error-check helpers
//------------------------------------------------------------------------------

bool check_balanced_counts(int *counts, int n);    // All counts ≈ equal
bool check_balanced_bits(int *ones, int N, int B); // Is each bit set with ~50% prob?

void check_rng_clean(randompack_rng *rng);         // rng ≠ 0 and last_error empty?
void check_success(bool ok, randompack_rng *rng);  // ok and last_error empty?
void check_failure(bool ok, randompack_rng *rng);  // !ok and last_error ≠ ""

randompack_rng *create_seeded_rng(const char *engine, int seed);

void check_u01_distribution(double *u, int n);
void check_u01_distributionf(float *u, int n);

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

static inline void testutil_silence_unused(void) {
  UNUSED(engines);
  UNUSED(abbrev);
  UNUSED(engine_table);
}
#endif
