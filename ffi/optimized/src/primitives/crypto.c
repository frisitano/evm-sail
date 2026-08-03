/* Generated hash and nominal-region adapters. This file is compiled per build
 * against that build's model header (EVMSAIL_MODEL_H), so generated aggregate
 * layouts are never mirrored by hand. */
#include "evmsail/prelude.h"
#include "evmsail/host/region_access.h"
#include "evmsail/primitives/crypto.h"
#include "host/state/runtime.h"
#include "primitives/value.h"
#include "zkvm_accelerators.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t hash_empty;

#define DEFINE_SLICE_HASH(name, slice_type, resolver, algorithm, digest_type) \
  Hash32 name(                                                   \
      struct slice_type input) {                                               \
    const uint64_t off = input.off;                                          \
    const uint64_t len = input.len;                                          \
    const uint8_t *bytes = resolver(off, len);                                 \
    Hash32 digest = {{0}};                            \
    _Static_assert(sizeof(digest) == sizeof(digest_type),                     \
                   "accelerator digest must be 32 bytes");                   \
    if (!bytes || len > UINT32_MAX ||                                         \
        algorithm(len ? bytes : &hash_empty, (size_t)len,                     \
                  (digest_type *)(void *)&digest) !=                          \
            ZKVM_EOK)                                                         \
      memset(&digest, 0, sizeof(digest));                                     \
    return digest;                                                            \
  }

DEFINE_SLICE_HASH(host_keccak_stateless_input,
                  StatelessInputSliceFields, stateless_input_ptr,
                  zkvm_keccak256, zkvm_keccak256_hash)
DEFINE_SLICE_HASH(host_keccak_scratch, ScratchSliceFields,
                  scratch_ptr, zkvm_keccak256, zkvm_keccak256_hash)
DEFINE_SLICE_HASH(host_keccak_memory, EvmMemorySliceFields, memory_ptr,
                  zkvm_keccak256, zkvm_keccak256_hash)
DEFINE_SLICE_HASH(host_keccak_code, CodeRegionSliceFields, code_ptr,
                  zkvm_keccak256, zkvm_keccak256_hash)
DEFINE_SLICE_HASH(host_keccak_output, OutputSliceFields, output_ptr,
                  zkvm_keccak256, zkvm_keccak256_hash)
DEFINE_SLICE_HASH(host_keccak_log_data, LogDataSliceFields,
                  log_data_ptr, zkvm_keccak256, zkvm_keccak256_hash)
DEFINE_SLICE_HASH(host_sha256_stateless_input,
                  StatelessInputSliceFields, stateless_input_ptr,
                  zkvm_sha256, zkvm_sha256_hash)
DEFINE_SLICE_HASH(host_sha256_scratch, ScratchSliceFields,
                  scratch_ptr, zkvm_sha256, zkvm_sha256_hash)
DEFINE_SLICE_HASH(host_sha256_memory, EvmMemorySliceFields, memory_ptr,
                  zkvm_sha256, zkvm_sha256_hash)

#undef DEFINE_SLICE_HASH

Hash32 host_keccak_word(const U256 input) {
  uint8_t bytes[32];
  Hash32 digest = {{0}};
  _Static_assert(sizeof(digest) == sizeof(zkvm_keccak256_hash),
                 "accelerator digest must be 32 bytes");
  sail_word_to_be_bytes(bytes, (input));
  if (zkvm_keccak256(bytes, sizeof(bytes),
                     (zkvm_keccak256_hash *)(void *)&digest) != ZKVM_EOK)
    memset(&digest, 0, sizeof(digest));
  return digest;
}

Hash32 host_keccak_address(Address input) {
  Hash32 digest = {{0}};
  _Static_assert(sizeof(digest) == sizeof(zkvm_keccak256_hash),
                 "accelerator digest must be 32 bytes");
  if (zkvm_keccak256(address_bytes_const(&input), 20,
                     (zkvm_keccak256_hash *)(void *)&digest) !=
      ZKVM_EOK)
    memset(&digest, 0, sizeof(digest));
  return digest;
}

Hash32 host_sha256_pair(
    Hash32 left, Hash32 right) {
  uint8_t bytes[64];
  Hash32 digest = {{0}};
  _Static_assert(sizeof(digest) == sizeof(zkvm_sha256_hash),
                 "accelerator digest must be 32 bytes");
  memcpy(bytes, hash_bytes_const(&left), 32);
  memcpy(bytes + 32, hash_bytes_const(&right), 32);
  if (zkvm_sha256(bytes, sizeof(bytes),
                  (zkvm_sha256_hash *)(void *)&digest) != ZKVM_EOK)
    memset(&digest, 0, sizeof(digest));
  return digest;
}

/* LOG payloads are copied from active-frame memory into the log-data arena. */
unit log_add_data_memory(struct EvmMemorySliceFields data) {
  const uint64_t off = data.off;
  const uint64_t len = data.len;
  const uint8_t *p = memory_ptr(off, len);
  if (p) log_add_data_bulk(p, len);
  return UNIT;
}
