// -*- C -*-
static int serialized_extra_bytes(rng_engine engine) {
  rng_entry *ent = find_entry(engine);
  if (!ent) return 0;
  return ent->extra_words * (int)sizeof(uint64_t);
}

static int serialized_need(randompack_rng *rng) {
  return STATE_MIN_NEED + serialized_extra_bytes(rng->engine);
}

// For deserialization: compute required size from a blob header
static int serialized_need_from_blob(const rng_blob *blob) {
  return STATE_MIN_NEED + serialized_extra_bytes((rng_engine)blob->engine);
}

static bool serialize(uint8_t *buf, int need, randompack_rng *rng) {
  (void)need;
  rng_blob blob = (rng_blob){0};
  blob.version = 1;
  blob.engine  = rng->engine;

  // Copy core 64-bit state
  for (int i = 0; i < LEN(rng->state.u64); i++)
    blob.state_u64[i] = rng->state.u64[i];

  blob.spare_norm = rng->spare_norm;
  blob.buf_word   = (uint32_t)rng->buf_word;
  blob.buf_byte   = (uint32_t)rng->buf_byte;
  blob.reserved0  = 0;
  blob.reserved1  = 0;

  memcpy(buf, &blob, STATE_MIN_NEED);
  if (rng->extra_state) {
    int extra = serialized_extra_bytes(rng->engine);
    if (extra > 0)
      memcpy(buf + STATE_MIN_NEED, rng->extra_state, (size_t)extra);
  }
  return true;
}

static bool deserialize(const rng_blob *blob, rng_entry *ent,
                        const uint8_t *extra_buf, int extra_bytes,
                        randompack_rng *rng) {
  rng->engine = (rng_engine)blob->engine;
  rng->next = ent->next;
  rng->fill = ent->fill;

  if (ent->extra_words > 0) {
    if (!rng->extra_state) {
      if (!ALLOC(rng->extra_state, ent->extra_words)) return false;
    }
    if (extra_bytes > 0 && extra_buf)
      memcpy(rng->extra_state, extra_buf, (size_t)extra_bytes);
  }

  for (int i = 0; i < LEN(rng->state.u64); i++)
    rng->state.u64[i] = blob->state_u64[i];
  rng->state.u32 = blob->state_u64[0];
  rng->buf_word = BUFFSIZE;
  rng->buf_byte = 0;
  rng->spare_norm = blob->spare_norm;

  return true;
}
