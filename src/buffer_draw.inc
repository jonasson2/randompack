// -*- C -*-
static void fill64_generic(uint64_t *out, int n, randompack_rng *rng) {
  for (int i=0; i<n; i++) out[i] = rng->next(rng);
}

static void fill64_x256ss(uint64_t *out, int n, randompack_rng *rng) {
  for (int i=0; i<n; i++) out[i] = nextss(rng);
}

static inline uint64_t draw_u64(randompack_rng *rng) {
  if (rng->buff_pos == BUFFSIZE) {
    rng->fill(rng->buff, BUFFSIZE, rng);
    rng->buff_pos = 0;
  }
  return rng->buff[rng->buff_pos++];
}

// Copy n fresh uint64 draws into out, using rng->buff as a refillable reservoir.
static inline void draw_u64_copy(uint64_t *out, int n, randompack_rng *rng) {
  int i = 0;
  while (i < n) {
    if (rng->buff_pos == BUFFSIZE) {
      rng->fill(rng->buff, BUFFSIZE, rng);
      rng->buff_pos = 0;
    }
    int take = min(n - i, BUFFSIZE - rng->buff_pos);
    copy64(out + i, rng->buff + rng->buff_pos, take);
    rng->buff_pos += take;
    i += take;
  }
}
