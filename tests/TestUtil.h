// Number of elements in an array (NOT a pointer)
#define VEC_LEN(a) ((int)(sizeof(a)/sizeof((a)[0])))

// Elementwise equality for whole arrays
#define CHECK_VEC_EQ(a, b)                      \
 do {                                           \
   int _n = VEC_LEN(a);                         \
   for (int _i=0; _i<_n; _i++)                  \
     xCheck((a)[_i] == (b)[_i]);                \
 } while (0)

// Check that arrays differ in at least one position
#define CHECK_VEC_NEQ(a, b)                     \
 do {                                           \
   int _n = VEC_LEN(a);                         \
   bool _diff = false;                          \
   for (int _i=0; _i<_n; _i++) {                \
     if ((a)[_i] != (b)[_i]) {                  \
       _diff = true;                            \
       break;                                   \
     }                                          \
   }                                            \
   xCheck(_diff);                               \
 } while (0)

#define RUN_TEST(x)        \
  do {                     \
    xCheckAddMsg(#x);      \
    test_##x();            \
  } while (0)

