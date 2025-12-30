// -*- C -*-
static inline void fill_buf(randompack_rng *rng) {
  rng->fill(rng);
  rng->buf_word = 0;
  rng->buf_byte = 0;
}

static inline void align16(randompack_rng *rng) { // align to next 2-byte boundary
  if (rng->buf_byte & 1) rng->buf_byte++;
  if (rng->buf_byte == 8) {
    rng->buf_byte = 0;
    rng->buf_word++;
  }
}

static inline void align32(randompack_rng *rng) { // align to next 4-byte boundary
  rng->buf_byte = (rng->buf_byte + 3)/4*4;
  if (rng->buf_byte == 8) {
    rng->buf_byte = 0;
    rng->buf_word++;
  }
}

static inline void align64(randompack_rng *rng) { // align to beginning or word
  if (rng->buf_byte > 0) {
    rng->buf_byte = 0;
    rng->buf_word++;
  }
}

static inline void fill_buf_if_empty(randompack_rng *rng) {
  if (rng->buf_word >= BUFSIZE) {
    rng->fill(rng);
    rng->buf_word = 0;
    rng->buf_byte = 0;
  }
}

static inline uint8_t draw_byte(randompack_rng *rng) {
  if (rng->buf_word >= BUFSIZE) fill_buf(rng);
  uint64_t w = rng->buf[rng->buf_word];
  uint8_t b = w >> (8 * rng->buf_byte);
  rng->buf_byte++;
  if (rng->buf_byte >= 8) {
    rng->buf_byte = 0;
    rng->buf_word++;
  }
  return b;
}

static inline uint32_t draw_u32(randompack_rng *rng) {
  int w = rng->buf_word;
  int b = rng->buf_byte;
  b = (b + 3)/4*4;
  if (b == 8) { // Align to next 32-bit boundary
    b = 0;
    w++;
  }
  if (w >= BUFSIZE) { // Refill if needed
    rng->fill(rng);
    w = 0;
    b = 0;
  }
  uint64_t word = rng->buf[w];
  uint32_t x = (b == 0) ? word : (word >> 32);
  b += 4;
  if (b == 8) { // Advance by one 32-bit chunk
    b = 0;
    w++;
  }
  rng->buf_word = w;
  rng->buf_byte = b;
  return x;
}

static inline uint64_t draw_u64(randompack_rng *rng) { // Fast single word
  int w = rng->buf_word;
  int b = rng->buf_byte;
  if (b != 0) {
    b = 0;
    w++;
  }
  if (w >= BUFSIZE) {
    rng->fill(rng);
    w = 0;
    b = 0;
  }
  rng->buf_word = w + 1;
  rng->buf_byte = b;
  return rng->buf[w];
}

static inline double draw_u01(randompack_rng *rng) {
  return U64_TO_DOUBLE(draw_u64(rng));
}

static inline double draw_u01f(randompack_rng *rng) {
  return U32_TO_FLOAT(draw_u32(rng));
}

// Copy n fresh uint16 draws into out using the shared byte buffer.
static inline void draw_u16_copy(uint16_t *out, size_t n, randompack_rng *rng) {
  align16(rng);
  uint16_t *buf16 = (uint16_t*)rng->buf;
  size_t i = 0;
  while (i < n) {
    fill_buf_if_empty(rng);
    int pos16 = 4*rng->buf_word + rng->buf_byte/2;
    int avail = 4*BUFSIZE - pos16;
    size_t rem = n - i;
    int take = avail;
    if (rem < (size_t)avail) take = (int)rem;
    copy16(out + i, buf16 + pos16, take);
    pos16 += take;
    rng->buf_word = pos16/4;
    rng->buf_byte = (pos16 & 3)*2;
    i += take;
  }
}

// Copy n fresh uint32 draws into out using the shared byte buffer.
static inline void draw_u32_copy(uint32_t *out, size_t n, randompack_rng *rng) {
  align32(rng);
  uint32_t *buf32 = (uint32_t*)rng->buf;
  size_t i = 0;
  while (i < n) {
    fill_buf_if_empty(rng);
    int pos32 = 2*rng->buf_word + rng->buf_byte/4;
    int avail = 2*BUFSIZE - pos32;
    size_t rem = n - i;
    int take = avail;
    if (rem < (size_t)avail) take = (int)rem;
    copy32(out + i, buf32 + pos32, take);
    pos32 += take;
    rng->buf_word = pos32/2;
    rng->buf_byte = (pos32 & 1)*4;
    i += take;
  }
}

// Copy n fresh uint64 draws into out using the shared byte buffer.
static inline void draw_u64_copy(uint64_t *out, size_t n, randompack_rng *rng) {
  align64(rng);
  size_t i = 0;
  while (i < n) {
    fill_buf_if_empty(rng);
    int take = min(n - i, BUFSIZE - rng->buf_word);
    copy64(out + i, rng->buf + rng->buf_word, take);
    rng->buf_word += take;
    i += take;
  }
}
