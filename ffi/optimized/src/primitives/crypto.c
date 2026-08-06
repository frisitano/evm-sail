/* Generated hash and nominal-region adapters. This file is compiled per build
 * against that build's model header (EVMSAIL_MODEL_H), so generated aggregate
 * layouts are never mirrored by hand. */
#include "evmsail/prelude.h"
#include "evmsail/host/region_access.h"
#include "evmsail/primitives/crypto.h"
#include "primitives/value.h"
#include "zkvm_accelerators.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t hash_empty = 0;

#define DEFINE_POINTER_SLICE_HASH(name, slice_type, algorithm, digest_type)                        \
  Hash32 name(struct slice_type input)                                                             \
  {                                                                                                \
    const uint64_t len = input.len;                                                                \
    const uint8_t *bytes = input.bytes;                                                            \
    digest_type native_digest = {{0}};                                                             \
    Hash32 digest = {{0}};                                                                         \
    _Static_assert(sizeof(digest) == sizeof(native_digest),                                        \
                   "accelerator digest must be 32 bytes");                                         \
    if ((!bytes && len != 0) ||                                                                    \
        algorithm(len ? bytes : &hash_empty, (size_t)len, &native_digest) != ZKVM_EOK)             \
      memset(&native_digest, 0, sizeof(native_digest));                                            \
    memcpy(&digest, &native_digest, sizeof digest);                                                \
    return digest;                                                                                 \
  }

DEFINE_POINTER_SLICE_HASH(host_keccak_stateless_input, StatelessInputSliceFields, zkvm_keccak256,
                          zkvm_keccak256_hash)
DEFINE_POINTER_SLICE_HASH(host_keccak_scratch, ScratchSliceFields, zkvm_keccak256,
                          zkvm_keccak256_hash)
DEFINE_POINTER_SLICE_HASH(host_keccak_memory, EvmMemorySliceFields, zkvm_keccak256,
                          zkvm_keccak256_hash)
DEFINE_POINTER_SLICE_HASH(host_keccak_output, OutputSliceFields, zkvm_keccak256,
                          zkvm_keccak256_hash)
DEFINE_POINTER_SLICE_HASH(host_keccak_log_data, LogDataSliceFields, zkvm_keccak256,
                          zkvm_keccak256_hash)
DEFINE_POINTER_SLICE_HASH(host_sha256_stateless_input, StatelessInputSliceFields, zkvm_sha256,
                          zkvm_sha256_hash)
DEFINE_POINTER_SLICE_HASH(host_sha256_scratch, ScratchSliceFields, zkvm_sha256, zkvm_sha256_hash)
DEFINE_POINTER_SLICE_HASH(host_sha256_memory, EvmMemorySliceFields, zkvm_sha256, zkvm_sha256_hash)
DEFINE_POINTER_SLICE_HASH(host_keccak_code, CodeRegionSliceFields, zkvm_keccak256,
                          zkvm_keccak256_hash)

#undef DEFINE_POINTER_SLICE_HASH

Hash32 host_keccak_word(const U256 input)
{
  uint8_t bytes[32];
  zkvm_keccak256_hash native_digest = {{0}};
  Hash32 digest = {{0}};
  _Static_assert(sizeof(digest) == sizeof(zkvm_keccak256_hash),
                 "accelerator digest must be 32 bytes");
  sail_word_to_be_bytes(bytes, input);
  if (zkvm_keccak256(bytes, sizeof(bytes), &native_digest) != ZKVM_EOK) {
    memset(&native_digest, 0, sizeof native_digest);
  }
  memcpy(&digest, &native_digest, sizeof digest);
  return digest;
}

Hash32 host_keccak_address(Address input)
{
  zkvm_keccak256_hash native_digest = {{0}};
  Hash32 digest = {{0}};
  _Static_assert(sizeof(digest) == sizeof(zkvm_keccak256_hash),
                 "accelerator digest must be 32 bytes");
  if (zkvm_keccak256(address_bytes_const(&input), 20, &native_digest) != ZKVM_EOK) {
    memset(&native_digest, 0, sizeof native_digest);
  }
  memcpy(&digest, &native_digest, sizeof digest);
  return digest;
}

Hash32 host_sha256_pair(Hash32 left, Hash32 right)
{
  uint8_t bytes[64];
  zkvm_sha256_hash native_digest = {{0}};
  Hash32 digest = {{0}};
  _Static_assert(sizeof(digest) == sizeof(zkvm_sha256_hash), "accelerator digest must be 32 bytes");
  memcpy(bytes, hash_bytes_const(&left), 32);
  memcpy(bytes + 32, hash_bytes_const(&right), 32);
  if (zkvm_sha256(bytes, sizeof(bytes), &native_digest) != ZKVM_EOK) {
    memset(&native_digest, 0, sizeof native_digest);
  }
  memcpy(&digest, &native_digest, sizeof digest);
  return digest;
}
