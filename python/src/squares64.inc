// -*- C -*-

static inline uint64_t squares64(uint64_t ctr, uint64_t key) {
  uint64_t x, y, z, t;

  y = x = ctr*key;
  z = y + key;

  x = x*x + y;  x = (x >> 32) | (x << 32);
  x = x*x + z;  x = (x >> 32) | (x << 32);
  x = x*x + y;  x = (x >> 32) | (x << 32);
  t = x = x*x + z;  x = (x >> 32) | (x << 32);

  return t ^ ((x*x + y) >> 32);
}

#define SQUARES_ROUND(outptr) do {              \
   y = x = ctr*key; z = y + key;                \
   x = x*x + y; x = (x >> 32) | (x << 32);      \
   x = x*x + z; x = (x >> 32) | (x << 32);      \
   x = x*x + y; x = (x >> 32) | (x << 32);      \
   t = x = x*x + z; x = (x >> 32) | (x << 32);  \
   outptr = t ^ ((x*x + y) >> 32);              \
   ctr++;                                       \
 } while(0)

static inline void fill_squares(randompack_rng *rng, size_t len) {
  uint64_t *out = rng->buf.u64 + rng->buf_word;
  uint64_t ctr = rng->state.u64[0];
  uint64_t key = rng->state.u64[1];
  uint64_t x, y, z, t;
  for (size_t i = 0; i < len; i += 4) {
    y = x = ctr*key; z = y + key;
    x = x*x + y;  x = (x >> 32) | (x << 32);
    x = x*x + z;  x = (x >> 32) | (x << 32);
    x = x*x + y;  x = (x >> 32) | (x << 32);
    t = x = x*x + z;  x = (x >> 32) | (x << 32);
    out[i] = t ^ ((x*x + y) >> 32);
    ctr++;
    y = x = ctr*key; z = y + key;
    x = x*x + y;  x = (x >> 32) | (x << 32);
    x = x*x + z;  x = (x >> 32) | (x << 32);
    x = x*x + y;  x = (x >> 32) | (x << 32);
    t = x = x*x + z;  x = (x >> 32) | (x << 32);
    out[i+1] = t ^ ((x*x + y) >> 32);
    ctr++;
    y = x = ctr*key; z = y + key;
    x = x*x + y;  x = (x >> 32) | (x << 32);
    x = x*x + z;  x = (x >> 32) | (x << 32);
    x = x*x + y;  x = (x >> 32) | (x << 32);
    t = x = x*x + z;  x = (x >> 32) | (x << 32);
    out[i+2] = t ^ ((x*x + y) >> 32);
    ctr++;
    y = x = ctr*key; z = y + key;
    x = x*x + y;  x = (x >> 32) | (x << 32);
    x = x*x + z;  x = (x >> 32) | (x << 32);
    x = x*x + y;  x = (x >> 32) | (x << 32);
    t = x = x*x + z;  x = (x >> 32) | (x << 32);
    out[i+3] = t ^ ((x*x + y) >> 32);
    ctr++;
  }
  rng->state.u64[0] = ctr;
}

static inline void fill_squares_cb(uint64_t buf[], size_t len, uint64_t ctr, uint64_t key) {
  uint64_t x, y, z, t;
  for (size_t i = 0; i < len; i++) {
	 y = x = ctr*key;
	 z = y + key;
	 x = x*x + y;  x = (x >> 32) | (x << 32);
	 x = x*x + z;  x = (x >> 32) | (x << 32);
	 x = x*x + y;  x = (x >> 32) | (x << 32);
	 t = x = x*x + z;  x = (x >> 32) | (x << 32);
	 buf[i] = t ^ ((x*x + y) >> 32);
	 ctr++;
  }
}
