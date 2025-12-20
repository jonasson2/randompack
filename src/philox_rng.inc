// -*- C -*-
// A stateful rng implementation of Philox. The counter is stored
// in rng->state and the key in rng->extra_state
typedef struct {
  randompack_philox_key key;
  int idx;
} philox_state;

static inline void fill_philox(randompack_rng *rng) {
  uint64_t *out = rng->buf;
  size_t n = BUFSIZE;
  randompack_counter ctr;
  randompack_philox_key key;
  copy64(ctr.v, rng->state.u64, 4);
  copy64(key.v, rng->state.u64 + 4, 2);
  size_t i = 0;
  while (i < n) {
    randompack_counter blk = philox4x64(ctr, key);
    int take = 4;
    size_t rem = n - i;
    if (rem < 4) take = (int)rem;
    copy64(out + i, blk.v, take);
    i += (size_t)take;
    ctr.v[0]++;
  }
  copy64(rng->state.u64, ctr.v, 4);
}
