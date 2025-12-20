// -*- C -*-
// For deserialization: compute required size from a blob header
static bool serialize(uint8_t *buf, int need, randompack_rng *rng) {
  (void)need;
  rng_blob blob = {0};
  blob.version = 1;
  blob.engine  = rng->engine;

  // Copy core 64-bit state
  copy64(blob.state_u64, rng->state.u64, LEN(rng->state.u64));
  blob.spare_norm = rng->spare_norm;
  blob.buf_word   = rng->buf_word;
  blob.buf_byte   = rng->buf_byte;
  blob.reserved0  = 0;
  blob.reserved1  = 0;
  memcpy(blob.buf, rng->buf, sizeof blob.buf);
  memcpy(buf, &blob, STATE_NEED);
  return true;
}

static bool deserialize(rng_blob *blob, rng_entry *ent, randompack_rng *rng) {
  rng->engine = blob->engine;
  rng->fill = ent->fill;
  copy64(rng->state.u64, blob->state_u64, LEN(rng->state.u64));
  copy64(rng->buf, blob->buf, BUFSIZE);
  rng->buf_word = blob->buf_word;
  rng->buf_byte = blob->buf_byte;
  rng->spare_norm = blob->spare_norm;
  return true;
}
