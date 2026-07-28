/* Generated hash and nominal-region adapters. This file is compiled per build
 * against that build's model header (EVMSAIL_MODEL_H), so generated aggregate
 * layouts are never mirrored by hand. */
#include EVMSAIL_MODEL_H
#include "region_access.h"
#include "hash_glue.h"
#include "kernel_state.h"
#include "value_convert.h"
#include "zkvm_accelerators.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t hash_empty;

#define DEFINE_SLICE_HASH(name, slice_type, resolver, algorithm, digest_type) \
  EVMSAIL_HASH_RETURN name(                                                   \
      EVMSAIL_HASH_RESULT(result) struct slice_type input) {                  \
    const uint64_t off = evmsail_byte_quantity_value(input.zoff);             \
    const uint64_t len = evmsail_byte_quantity_value(input.zlen);             \
    const uint8_t *bytes = resolver(off, len);                                \
    digest_type digest = {{0}};                                               \
    if (!bytes || len > UINT32_MAX ||                                         \
        algorithm(len ? bytes : &hash_empty, (size_t)len, &digest) !=         \
            ZKVM_EOK)                                                         \
      memset(&digest, 0, sizeof(digest));                                     \
    EVMSAIL_RETURN_HASH_BE_BYTES(result, digest.data);                        \
  }

DEFINE_SLICE_HASH(host_keccak_stateless_input,
                  zStatelessInputSliceFields, evmsail_stateless_input_ptr,
                  zkvm_keccak256, zkvm_keccak256_hash)
DEFINE_SLICE_HASH(host_keccak_scratch, zScratchSliceFields,
                  evmsail_scratch_ptr, zkvm_keccak256, zkvm_keccak256_hash)
DEFINE_SLICE_HASH(host_keccak_memory, zMemorySliceFields, evmsail_memory_ptr,
                  zkvm_keccak256, zkvm_keccak256_hash)
DEFINE_SLICE_HASH(host_keccak_code, zCodeRegionSliceFields, evmsail_code_ptr,
                  zkvm_keccak256, zkvm_keccak256_hash)
DEFINE_SLICE_HASH(host_keccak_output, zOutputSliceFields, evmsail_output_ptr,
                  zkvm_keccak256, zkvm_keccak256_hash)
DEFINE_SLICE_HASH(host_keccak_log_data, zLogDataSliceFields,
                  evmsail_log_data_ptr, zkvm_keccak256, zkvm_keccak256_hash)
DEFINE_SLICE_HASH(host_sha256_stateless_input,
                  zStatelessInputSliceFields, evmsail_stateless_input_ptr,
                  zkvm_sha256, zkvm_sha256_hash)
DEFINE_SLICE_HASH(host_sha256_scratch, zScratchSliceFields,
                  evmsail_scratch_ptr, zkvm_sha256, zkvm_sha256_hash)
DEFINE_SLICE_HASH(host_sha256_memory, zMemorySliceFields, evmsail_memory_ptr,
                  zkvm_sha256, zkvm_sha256_hash)

#undef DEFINE_SLICE_HASH

EVMSAIL_HASH_RETURN host_keccak_word(
    EVMSAIL_HASH_RESULT(result) EVMSAIL_WORD_PARAM(input)) {
  uint8_t bytes[32];
  zkvm_keccak256_hash digest = {{0}};
  sail_word_to_be_bytes(bytes, EVMSAIL_WORD_VALUE(input));
  if (zkvm_keccak256(bytes, sizeof(bytes), &digest) != ZKVM_EOK)
    memset(&digest, 0, sizeof(digest));
  EVMSAIL_RETURN_HASH_BE_BYTES(result, digest.data);
}

EVMSAIL_HASH_RETURN host_keccak_address(
    EVMSAIL_HASH_RESULT(result) sail_address input) {
  zkvm_keccak256_hash digest = {{0}};
  if (zkvm_keccak256(input.bytes, sizeof(input.bytes), &digest) != ZKVM_EOK)
    memset(&digest, 0, sizeof(digest));
  EVMSAIL_RETURN_HASH_BE_BYTES(result, digest.data);
}

EVMSAIL_HASH_RETURN host_sha256_pair(
    EVMSAIL_HASH_RESULT(result) sail_hash left, sail_hash right) {
  uint8_t bytes[64];
  zkvm_sha256_hash digest = {{0}};
  memcpy(bytes, left.bytes, sizeof(left.bytes));
  memcpy(bytes + sizeof(left.bytes), right.bytes, sizeof(right.bytes));
  if (zkvm_sha256(bytes, sizeof(bytes), &digest) != ZKVM_EOK)
    memset(&digest, 0, sizeof(digest));
  EVMSAIL_RETURN_HASH_BE_BYTES(result, digest.data);
}

/* LOG payloads are copied from active-frame memory into the log-data arena. */
unit log_add_data_memory(struct zMemorySliceFields data) {
  const uint64_t off = evmsail_byte_quantity_value(data.zoff);
  const uint64_t len = evmsail_byte_quantity_value(data.zlen);
  const uint8_t *p = evmsail_memory_ptr(off, len);
  if (p) log_add_data_bulk(p, len);
  return UNIT;
}
