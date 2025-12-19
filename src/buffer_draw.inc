// -*- C -*-
static void fill64_generic(uint64_t *out, size_t n, randompack_rng *rng) {
  for (size_t i = 0; i < n; i++) out[i] = rng->next(rng);
}

static inline void fill_buf(randompack_rng *rng) {
  rng->fill(rng->buf, BUFFSIZE, rng);
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
  if (rng->buf_word >= BUFFSIZE) {
    rng->fill(rng->buf, BUFFSIZE, rng);
    rng->buf_word = 0;
    rng->buf_byte = 0;
  }
}

static inline uint8_t draw_byte(randompack_rng *rng) {
  if (rng->buf_word >= BUFFSIZE) fill_buf(rng);
  uint64_t w = rng->buf[rng->buf_word];
  uint8_t b = (uint8_t)(w >> (8 * rng->buf_byte));
  rng->buf_byte++;
  if (rng->buf_byte >= 8) {
    rng->buf_byte = 0;
    rng->buf_word++;
  }
  return b;
}

static inline uint16_t rand_u16(randompack_rng *rng) {
  align16(rng);
  fill_buf_if_empty(rng);
  uint64_t w = rng->buf[rng->buf_word];
  uint16_t x = (uint16_t)(w >> (8*rng->buf_byte));
  rng->buf_byte += 2;
  if (rng->buf_byte == 8) {
    rng->buf_byte = 0;
    rng->buf_word++;
  }
  return x;
}

static inline uint32_t draw_u32(randompack_rng *rng) {
  align32(rng);
  fill_buf_if_empty(rng);
  uint64_t w = rng->buf[rng->buf_word];
  if (rng->buf_byte) { // buf_byte == 4 => high 32 bits
    rng->buf_byte = 0;
    rng->buf_word++;
    return (uint32_t)(w >> 32);
  }
  rng->buf_byte = 4;   // low 32 bits
  return (uint32_t)w;
}

static inline uint64_t draw_u64(randompack_rng *rng) {
  fill_buf_if_empty(rng);
  return rng->buf[rng->buf_word++];
}

// Copy n fresh uint16 draws into out using the shared byte buffer.
static inline void draw_u16_copy(uint16_t *out, size_t n, randompack_rng *rng) {
  align16(rng);
  uint16_t *buf16 = (uint16_t*)rng->buf;
  size_t i = 0;
  while (i < n) {
    fill_buf_if_empty(rng);
    int pos16 = 4*rng->buf_word + rng->buf_byte/2;
    int avail = 4*BUFFSIZE - pos16;
    size_t rem = n - i;
    int take = avail;
    if (rem < (size_t)avail) take = (int)rem;
    copy16(out + i, buf16 + pos16, take);
    pos16 += take;
    rng->buf_word = pos16/4;
    rng->buf_byte = (pos16 & 3)*2;
    i += (size_t)take;
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
    int avail = 2*BUFFSIZE - pos32;
    size_t rem = n - i;
    int take = avail;
    if (rem < (size_t)avail) take = rem;
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
    int take = min(n - i, BUFFSIZE - rng->buf_word);
    copy64(out + i, rng->buf + rng->buf_word, take);
    rng->buf_word += take;
    i += take;
  }
}
