#!/usr/bin/env python3

import argparse
import ctypes
import struct
import sys
from pathlib import Path


RNG_STATE_WORDS = 9
STATE_PREFIX = 8 + 8*RNG_STATE_WORDS + 16

RFC_KEY = bytes.fromhex(
    "00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f "
    "10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f"
)
RFC_NONCE = bytes.fromhex("00 00 00 09 00 00 00 4a 00 00 00 00")
RFC_BLOCK = bytes.fromhex(
    "10 f1 e7 e4 d1 3b 59 15 50 0f dd 1f a3 20 71 c4 "
    "c7 d1 f4 c7 33 c0 68 03 04 22 aa 9a c3 d4 6c 4e "
    "d2 82 64 46 07 9f aa 09 14 c2 d7 05 d9 8b 02 a2 "
    "b5 12 9c d1 de 16 4e b9 cb d0 83 e8 a2 50 3c 4e"
)


def rotl32(x, n):
  return ((x << n) | (x >> (32 - n))) & 0xffffffff


def qround(state, a, b, c, d):
  state[a] = (state[a] + state[b]) & 0xffffffff
  state[d] ^= state[a]
  state[d] = rotl32(state[d], 16)
  state[c] = (state[c] + state[d]) & 0xffffffff
  state[b] ^= state[c]
  state[b] = rotl32(state[b], 12)
  state[a] = (state[a] + state[b]) & 0xffffffff
  state[d] ^= state[a]
  state[d] = rotl32(state[d], 8)
  state[c] = (state[c] + state[d]) & 0xffffffff
  state[b] ^= state[c]
  state[b] = rotl32(state[b], 7)


def chacha20_block(key, nonce, counter):
  assert len(key) == 32
  assert len(nonce) == 12
  consts = [0x61707865, 0x3320646e, 0x79622d32, 0x6b206574]
  key_words = list(struct.unpack("<8I", key))
  nonce_words = list(struct.unpack("<3I", nonce))
  start = consts + key_words + [counter] + nonce_words
  work = start.copy()
  for _ in range(10):
    qround(work, 0, 4, 8, 12)
    qround(work, 1, 5, 9, 13)
    qround(work, 2, 6, 10, 14)
    qround(work, 3, 7, 11, 15)
    qround(work, 0, 5, 10, 15)
    qround(work, 1, 6, 11, 12)
    qround(work, 2, 7, 8, 13)
    qround(work, 3, 4, 9, 14)
  out = [(work[i] + start[i]) & 0xffffffff for i in range(16)]
  return struct.pack("<16I", *out)


def chacha20_stream(key, nonce, counter, nbytes):
  out = bytearray()
  while len(out) < nbytes:
    out.extend(chacha20_block(key, nonce, counter))
    counter = (counter + 1) & 0xffffffff
  return bytes(out[:nbytes])


def check_reference():
  got = chacha20_block(RFC_KEY, RFC_NONCE, 1)
  if got != RFC_BLOCK:
    raise SystemExit("python reference failed RFC 8439 block test")


def default_lib_path():
  root = Path(__file__).resolve().parents[2]
  libdir = root / "install" / "lib"
  for name in ("librandompack.dylib", "librandompack.so", "randompack.dll"):
    path = libdir / name
    if path.exists():
      return path
  raise SystemExit("could not find installed randompack library in install/lib")


def load_randompack(libpath):
  lib = ctypes.CDLL(str(libpath))
  rng_p = ctypes.c_void_p
  lib.randompack_create.argtypes = [ctypes.c_char_p]
  lib.randompack_create.restype = rng_p
  lib.randompack_free.argtypes = [rng_p]
  lib.randompack_free.restype = None
  lib.randompack_seed.argtypes = [
      ctypes.c_int, ctypes.POINTER(ctypes.c_uint32), ctypes.c_int, rng_p
  ]
  lib.randompack_seed.restype = ctypes.c_bool
  lib.randompack_serialize.argtypes = [
      ctypes.POINTER(ctypes.c_uint8), ctypes.POINTER(ctypes.c_int), rng_p
  ]
  lib.randompack_serialize.restype = ctypes.c_bool
  lib.randompack_uint64.argtypes = [
      ctypes.POINTER(ctypes.c_uint64), ctypes.c_size_t, ctypes.c_uint64, rng_p
  ]
  lib.randompack_uint64.restype = ctypes.c_bool
  lib.randompack_last_error.argtypes = [rng_p]
  lib.randompack_last_error.restype = ctypes.c_char_p
  return lib


def check_ok(ok, lib, rng, what):
  if ok:
    return
  msg = lib.randompack_last_error(rng)
  if msg:
    raise SystemExit(f"{what}: {msg.decode()}")
  raise SystemExit(f"{what} failed")


def serialize_rng(lib, rng):
  need = ctypes.c_int(0)
  check_ok(lib.randompack_serialize(None, ctypes.byref(need), rng), lib, rng,
           "serialize size query")
  if need.value < STATE_PREFIX or (need.value - STATE_PREFIX) % 8 != 0:
    raise SystemExit(f"unexpected serialized size {need.value}")
  buf = (ctypes.c_uint8 * need.value)()
  check_ok(lib.randompack_serialize(buf, ctypes.byref(need), rng), lib, rng,
           "serialize")
  return bytes(buf)


def randompack_bytes(lib, rng, nbytes):
  nwords = (nbytes + 7) // 8
  words = (ctypes.c_uint64 * nwords)()
  check_ok(lib.randompack_uint64(words, nwords, 0, rng), lib, rng,
           "randompack_uint64")
  out = bytearray()
  for i in range(nwords):
    out.extend(struct.pack("<Q", words[i]))
  return bytes(out[:nbytes])


def extract_chacha_state(blob):
  state_bytes = blob[8:8 + 48]
  key = state_bytes[:32]
  nonce = state_bytes[32:44]
  counter = struct.unpack_from("<I", state_bytes, 44)[0]
  return key, nonce, counter


def first_mismatch(a, b):
  for i, (x, y) in enumerate(zip(a, b)):
    if x != y:
      return i
  return -1


def main():
  parser = argparse.ArgumentParser(
      description="Compare randompack chacha20 against a Python RFC 8439 reference"
  )
  parser.add_argument("length", type=int, nargs="?", default=256,
                      help="number of output bytes to compare")
  parser.add_argument("-s", "--seed", type=int, default=27,
                      help="randompack seed")
  parser.add_argument("--lib", type=Path, default=None,
                      help="path to librandompack")
  args = parser.parse_args()

  if args.length < 0:
    raise SystemExit("length must be nonnegative")

  check_reference()
  lib = load_randompack(args.lib or default_lib_path())
  rng = lib.randompack_create(b"chacha20")
  if not rng:
    raise SystemExit("could not create chacha20 rng")
  try:
    check_ok(lib.randompack_seed(args.seed, None, 0, rng), lib, rng,
             "randompack_seed")
    blob = serialize_rng(lib, rng)
    key, nonce, counter = extract_chacha_state(blob)
    got = randompack_bytes(lib, rng, args.length)
    want = chacha20_stream(key, nonce, counter, args.length)
    if got != want:
      i = first_mismatch(got, want)
      print(f"mismatch at byte {i}")
      print(f"randompack[{i}] = {got[i]:02x}")
      print(f"python    [{i}] = {want[i]:02x}")
      return 1
    print("match")
    print(f"seed    = {args.seed}")
    print(f"length  = {args.length}")
    print(f"counter = {counter}")
    print(f"key     = {key.hex()}")
    print(f"nonce   = {nonce.hex()}")
    return 0
  finally:
    lib.randompack_free(rng)


if __name__ == "__main__":
  sys.exit(main())
