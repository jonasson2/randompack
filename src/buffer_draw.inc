// -*- C -*-
ALWAYS_INLINE void fill_buf_if_empty(randompack_rng *rng) {
  if (rng->buf_word >= BUFSIZE) {
    rng->buf_word = rng->buf_byte = 0;
    rng->fill(rng, BUFSIZE);
  }
}

ALWAYS_INLINE void fill_if_empty(randompack_rng *rng, int *w) {
  if (*w >= BUFSIZE) {
    *w = rng->buf_word = 0;
    rng->fill(rng, BUFSIZE);
  }
}

ALWAYS_INLINE void fill_if_empty32(randompack_rng *rng, int *w) {
  if (*w >= 2*BUFSIZE) {
    *w = rng->buf_word = rng->buf_byte = 0;
    rng->fill(rng, BUFSIZE);
  }
}

ALWAYS_INLINE void make_half_full(randompack_rng *rng, int *w) {
  if (*w >= BUFSIZE/2) {
    *w = rng->buf_word = *w - BUFSIZE/2;
    rng->fill(rng, BUFSIZE/2);
  }
}

ALWAYS_INLINE void make_half_full32(randompack_rng *rng, int *w) {
  if (*w >= BUFSIZE) {
    *w -= BUFSIZE;
    rng->buf_word = *w/2;
    rng->buf_byte = (*w & 1)*4;
    rng->fill(rng, BUFSIZE/2);
  }
}

ALWAYS_INLINE void align16(randompack_rng *rng) { // align to next 2-byte boundary
  if (rng->buf_byte & 1) rng->buf_byte++;
  if (rng->buf_byte == 8) {
    rng->buf_byte = 0;
    rng->buf_word++;
  }
}

ALWAYS_INLINE void align32(randompack_rng *rng) { // align to next 4-byte boundary
  rng->buf_byte = (rng->buf_byte + 3)/4*4;
  if (rng->buf_byte == 8) {
    rng->buf_byte = 0;
    rng->buf_word++;
  }
}

ALWAYS_INLINE void align64(randompack_rng *rng) { // align to beginning or word
  if (rng->buf_byte > 0) {
    rng->buf_byte = 0;
    rng->buf_word++;
  }
}

ALWAYS_INLINE uint8_t draw_byte(randompack_rng *rng) {
  fill_buf_if_empty(rng);
  uint64_t w = rng->buf.u64[rng->buf_word];
  uint8_t b = w >> (8 * rng->buf_byte);
  rng->buf_byte++;
  if (rng->buf_byte >= 8) {
    rng->buf_byte = 0;
    rng->buf_word++;
  }
  return b;
}

ALWAYS_INLINE uint16_t draw_u16(randompack_rng *rng) {
  align16(rng);
  fill_buf_if_empty(rng);
  uint64_t w = rng->buf.u64[rng->buf_word];
  uint16_t v = (uint16_t)(w >> (8*rng->buf_byte));
  rng->buf_byte += 2;
  if (rng->buf_byte >= 8) {
    rng->buf_byte = 0;
    rng->buf_word++;
  }
  return v;
}

ALWAYS_INLINE uint32_t draw_u32(randompack_rng *rng, int *w) {
  if (*w >= 2*BUFSIZE) {
    *w = rng->buf_word = 0;
    rng->fill(rng, BUFSIZE);
  }
  return rng->buf.u32[(*w)++];
}

ALWAYS_INLINE uint64_t draw_u64(randompack_rng *rng, int *w) {
  if (*w >= BUFSIZE) {
    *w = rng->buf_word = 0;
    rng->fill(rng, BUFSIZE);
  }
  return rng->buf.u64[(*w)++];
}

ALWAYS_INLINE uint64_t draw_u64_nocheck(randompack_rng *rng, int *w) {
  return rng->buf.u64[(*w)++];
}

ALWAYS_INLINE uint32_t draw_u32_nocheck(randompack_rng *rng, int *w) {
  return rng->buf.u32[(*w)++];
}

ALWAYS_INLINE double draw_u01(randompack_rng *rng, int *w) {
  return U64_TO_DOUBLE(draw_u64(rng, w));
}

ALWAYS_INLINE float draw_u01f(randompack_rng *rng, int *w) {
  return U32_TO_FLOAT(draw_u32(rng, w));
}

static inline int enter_u32_mode(randompack_rng *rng) {
  align32(rng);
  return 2*rng->buf_word + rng->buf_byte/4;
}

static inline int enter_u64_mode(randompack_rng *rng) {
  align64(rng);
  return rng->buf_word;
}

static inline void exit_u64_mode(randompack_rng *rng, int w) {
  rng->buf_word = w;
}

static inline void exit_u32_mode(randompack_rng *rng, int w) {
  rng->buf_word = w/2;
  rng->buf_byte = (w & 1)*4;
}

// Copy n fresh uint16 draws into out using the shared byte buffer.
static inline void draw_u16_copy(uint16_t *out, size_t n, randompack_rng *rng) {
  align16(rng);
  uint16_t *buf16 = (uint16_t*)rng->buf.u64;
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
  uint32_t *buf32 = (uint32_t*)rng->buf.u64;
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
    copy64(out + i, rng->buf.u64 + rng->buf_word, take);
    rng->buf_word += take;
    i += take;
  }
}
