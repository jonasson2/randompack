// -*- C -*-
// A stateful rng implementation of Philox. The counter is stored
// in rng->state and the key + cached output in rng->extra_state
typedef struct {
  randompack_philox_key key;     // key.v[0..1]
  uint64_t              out[4];  // cached block of 4 outputs
  int                   idx;     // next word index in out, 0..4
} philox_state;

static uint64_t next_philox(randompack_rng *rng) {
  philox_state *st = (philox_state *)rng->extra_state;
  if (st->idx >= 4) {  // get next 4 words
    randompack_counter ctr = *(randompack_counter *)rng->state.u64;
    rand_phil_64bits(st->out, 4, ctr, st->key);
    rng->state.u64[0]++;
    st->idx = 0;
  }
  return st->out[st->idx++];
}
