// Various utilities for the test functions
#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include "Util.h"
#include "randompack.h"
#include "randompack_config.h"
#include "cephes.h"

// –––––––––––––––––––––––––––––––––––––––––––
// Macros to check determinism, e.g.:
//   TEST_DETERMINISM0("pcg64", u01)
//   TEST_DETERMINISM1("pcg64", exp, 2)
//   TEST_DETERMINISM2("pcg64", gamma, 0.5, 2)
// –––––––––––––––––––––––––––––––––––––––––––
#define DRAW(engine, seed, CALL) do {									\
	randompack_rng *rng = create_seeded_rng((engine), (seed));	\
	check_rng_clean(rng);													\
	bool ok = (CALL);															\
	check_success(ok, rng);													\
	randompack_free(rng);													\
 } while (0)

#define TEST_DETERMINISM0(engine, FUNC) do {										\
	double a[3], b[3], c[3];															\
	int n = 3;																				\
	DRAW((engine), 42, randompack_##FUNC(a, n, rng));							\
	DRAW((engine), 42, randompack_##FUNC(b, n, rng));							\
	DRAW((engine), 43, randompack_##FUNC(c, n, rng));							\
	xCheckMsg(equal_vecd(a, b, 3), (engine));										\
	xCheckMsg(a[0] != c[0] || a[1] != c[1] || a[2] != c[2], (engine));	\
 } while (0)

#define TEST_DETERMINISM1(engine, FUNC, p1) do {								\
	double a[3], b[3], c[3];															\
	int n = 3;																				\
	DRAW((engine), 42, randompack_##FUNC(a, n, (p1), rng));					\
	DRAW((engine), 42, randompack_##FUNC(b, n, (p1), rng));					\
	DRAW((engine), 43, randompack_##FUNC(c, n, (p1), rng));					\
	xCheckMsg(equal_vecd(a, b, 3), (engine));										\
	xCheckMsg(a[0] != c[0] || a[1] != c[1] || a[2] != c[2], (engine));	\
 } while (0)

#define TEST_DETERMINISM2(engine, FUNC, p1, p2) do {							\
	double a[3], b[3], c[3];															\
	int n = 3;																				\
	DRAW((engine), 42, randompack_##FUNC(a, n, (p1), (p2), rng));			\
	DRAW((engine), 42, randompack_##FUNC(b, n, (p1), (p2), rng));			\
	DRAW((engine), 43, randompack_##FUNC(c, n, (p1), (p2), rng));			\
	xCheckMsg(equal_vecd(a, b, 3), (engine));										\
	xCheckMsg(a[0] != c[0] || a[1] != c[1] || a[2] != c[2], (engine));	\
 } while (0)

#define TEST_EDGE_CASES0(engine, FUNC) do {								\
	double buf[4] = {0.1, 0.2, 0.3};											\
	double orig[4] = {0.1, 0.2, 0.3};										\
	bool ok;																			\
	randompack_rng *rng = create_seeded_rng((engine), 123);			\
	ok = randompack_##FUNC(buf, 0, rng); check_success(ok, rng);	\
	xCheck(equal_vecd(buf, orig, 4));										\
	ok = randompack_##FUNC(0, 4, rng); check_failure(ok, rng);		\
	ok = randompack_##FUNC(buf, 4, 0); xCheck(!ok);						\
	randompack_free(rng);														\
 } while (0)

#define TEST_EDGE_CASES1(engine, FUNC, p1) do {									\
	double buf[4] = {0.1, 0.2, 0.3};													\
	double orig[4] = {0.1, 0.2, 0.3};												\
	bool ok;																					\
	randompack_rng *rng = create_seeded_rng((engine), 123);					\
	ok = randompack_##FUNC(buf, 0, (p1), rng); check_success(ok, rng);	\
	xCheck(equal_vecd(buf, orig, 4));												\
	ok = randompack_##FUNC(0, 4, (p1), rng); check_failure(ok, rng);		\
	ok = randompack_##FUNC(buf, 4, (p1), 0); xCheck(!ok);						\
	randompack_free(rng);																\
 } while (0)

#define TEST_EDGE_CASES2(engine, FUNC, p1, p2) do {									\
	double buf[4] = {0.1, 0.2, 0.3};															\
	double orig[4] = {0.1, 0.2, 0.3};														\
	bool ok;																							\
	randompack_rng *rng = create_seeded_rng((engine), 123);							\
	ok = randompack_##FUNC(buf, 0, (p1), (p2), rng); check_success(ok, rng);	\
	xCheck(equal_vecd(buf, orig, 4));														\
	ok = randompack_##FUNC(0, 4, (p1), (p2), rng); check_failure(ok, rng);		\
	ok = randompack_##FUNC(buf, 4, (p1), (p2), 0); xCheck(!ok);						\
	randompack_free(rng);																		\
 } while (0)

#define TEST_ILLEGAL_PARAMS1(engine, FUNC, bad1) do {        \
  double buf[4];                                             \
  randompack_rng *rng = create_seeded_rng((engine), 123);    \
  bool ok = randompack_##FUNC(buf, 4, (bad1), rng);          \
  check_failure(ok, rng);                                    \
  randompack_free(rng);                                      \
} while (0)

#define TEST_ILLEGAL_PARAMS2(engine, FUNC, bad1, bad2) do {  \
  double buf[4];                                             \
  randompack_rng *rng = create_seeded_rng((engine), 123);    \
  bool ok = randompack_##FUNC(buf, 4, (bad1), (bad2), rng);  \
  check_failure(ok, rng);                                    \
  randompack_free(rng);                                      \
 } while (0)

//------------------------------------------------------------------------------
// Engine name tables used by tests
//------------------------------------------------------------------------------
static char *engines[] = {
  "xorshift128+",
  "xoshiro256**",
  "xoshiro256++",
  "chacha20",
  "philox"
#ifdef HAVE128
  , "pcg64_dxsm"
#endif
};

static char *abbrev[] = {
  "x128+",
  "x256**",
  "x256++",
  "chacha20",
  "philox"
#ifdef HAVE128
  , "pcg64"
#endif
};

typedef struct {
  const char *name;
  int state_words;
} engine_table_entry;

static engine_table_entry engine_table[] = {
  {"xorshift128+",  2},
  {"xoshiro256**",  4},
  {"xoshiro256++",  4},
  {"chacha20",      6},
  {"philox",        6},
#ifdef HAVE128
  {"pcg64_dxsm",    4},
#endif
};
//------------------------------------------------------------------------------
// Global test sizes / sample counts
//------------------------------------------------------------------------------

enum {
  N_BAL_CNTS = 500000,
  N_BAL_BITS = 40000,
  N_statistics = 100000
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

bool check_u01_distribution(double *u, int n);
#endif
