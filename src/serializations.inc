// -*- C -*-
static int serialized_extra_bytes(rng_engine engine) {
  int extra = 0;
  if (engine == CHACHA20)
    extra += sizeof(ChaCha20_Ctx);
  // later: if you add Philox extra_state to rng_blob,
  // you can extend this:
  // else if (engine == PHILOX)
  //   extra += sizeof(philox_state);
  return extra;
}

static int serialized_need(randompack_rng *rng) {
  return STATE_MIN_NEED + serialized_extra_bytes(rng->engine);
}

// For deserialization: compute required size from a blob header
static int serialized_need_from_blob(const rng_blob *blob) {
  return STATE_MIN_NEED + serialized_extra_bytes((rng_engine)blob->engine);
}

static int serialized_need(randompack_rng *rng) {
   int need = STATE_MIN_NEED;
 if (rng->engine == CHACHA20)
    need += sizeof(ChaCha20_Ctx);
  return need;
}

static bool serialize(uint8_t *buf, int need, randompack_rng *rng) {
  rng_blob blob = (rng_blob){0};
  blob.version = 1;
  blob.engine  = rng->engine;

  // Copy core 64-bit state
  for (int i = 0; i < LEN(rng->state.u64); i++)
    blob.state_u64[i] = rng->state.u64[i];

  blob.buf64      = rng->buf64;
  blob.spare_norm = rng->spare_norm;
  blob.reserved   = 0;

  // Copy CHACHA state if present
  if (rng->engine == CHACHA20 && rng->extra_state)
    memcpy(&blob.chacha, rng->extra_state, sizeof(ChaCha20_Ctx));

  // Write exactly 'need' bytes to buf
  memcpy(buf, &blob, need);
  return true;
}

static bool deserialize(const rng_blob *blob, rng_entry *ent, randompack_rng *rng) {
  rng->engine = (rng_engine)blob->engine;
  rng->next = ent->next;

  if (rng->engine == CHACHA20) {
    if (!rng->extra_state) {
      ChaCha20_Ctx *ctx;
      if (!ALLOC(ctx, 1)) return false;
      rng->extra_state = ctx;
    }
    memcpy(rng->extra_state, &blob->chacha, sizeof(ChaCha20_Ctx));
  }

  for (int i = 0; i < LEN(rng->state.u64); i++)
    rng->state.u64[i] = blob->state_u64[i];
  rng->state.u32 = blob->state_u64[0];
  rng->buf64 = blob->buf64;
  rng->spare_norm = blob->spare_norm;

  return true;
}
