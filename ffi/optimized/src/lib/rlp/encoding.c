/*
 * Private primitives shared by optimized high-level RLP encoders.
 *
 * Each operation computes its exact encoded length, borrows that many bytes
 * from the shared scratch arena, writes the preimage once in forward order,
 * and invokes the accelerator once. The borrow does not change Sail's visible
 * scratch cursor. The standard build retains the explicit Sail equations.
 */
#include "evmsail/prelude.h"

#include "lib/rlp/encoding.h"
#include "primitives/hash.h"
#include "evmsail/primitives/chain_config.h"
#include "evmsail/host/region_access.h"
#include "evmsail/host/scratch.h"
#include "primitives/value.h"
#include "zkvm_accelerators.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const uint8_t preimage_empty;

Hash32 zero_hash(void) {
  Hash32 result = {{0}};
  return result;
}

Hash32 keccak_bytes(
    const uint8_t *bytes, uint64_t len) {
  Hash32 digest = {{0}};
  _Static_assert(sizeof(digest) == sizeof(zkvm_keccak256_hash),
                 "accelerator digest must be 32 bytes");
  if (len > UINT32_MAX ||
      zkvm_keccak256(len ? bytes : &preimage_empty, (size_t)len,
                     (zkvm_keccak256_hash *)(void *)&digest) !=
          ZKVM_EOK)
    return zero_hash();
  return digest;
}

Hash32 sha256_bytes(
    const uint8_t *bytes, uint64_t len) {
  Hash32 digest = {{0}};
  _Static_assert(sizeof(digest) == sizeof(zkvm_sha256_hash),
                 "accelerator digest must be 32 bytes");
  if (len > UINT32_MAX ||
      zkvm_sha256(len ? bytes : &preimage_empty, (size_t)len,
                  (zkvm_sha256_hash *)(void *)&digest) !=
          ZKVM_EOK)
    return zero_hash();
  return digest;
}

bool rlp_add_length(uint64_t *total, uint64_t amount) {
  if (amount > UINT64_MAX - *total)
    return false;
  *total += amount;
  return true;
}

uint64_t rlp_length_byte_native(uint8_t value) {
  return value;
}

static unsigned be_u64_width(uint64_t value) {
  unsigned width = 0;
  while (value != 0) {
    ++width;
    value >>= 8;
  }
  return width;
}

static unsigned u256_width(U256 value) {
  uint8_t bytes[32];
  sail_word_to_be_bytes(bytes, value);
  unsigned first = 0;
  while (first < sizeof(bytes) && bytes[first] == 0)
    ++first;
  return (unsigned)sizeof(bytes) - first;
}

uint64_t rlp_quantity_size_u64(uint64_t value) {
  if (value == 0 || value < 0x80)
    return 1;
  return 1 + be_u64_width(value);
}

uint64_t rlp_quantity_size_u256(U256 value) {
  const unsigned width = u256_width(value);
  if (width == 0)
    return 1;
  if (width == 1 && value.limbs[0] < 0x80)
    return 1;
  return 1 + width;
}

static unsigned rlp_length_width(uint64_t len) {
  return be_u64_width(len);
}

static uint64_t rlp_string_prefix_size(uint64_t len, uint8_t first) {
  if (len == 1 && first < 0x80)
    return 0;
  if (len <= 55)
    return 1;
  return 1 + rlp_length_width(len);
}

uint64_t rlp_string_size(uint64_t len, uint8_t first) {
  uint64_t result = len;
  if (!rlp_add_length(&result, rlp_string_prefix_size(len, first)))
    return UINT64_MAX;
  return result;
}

uint64_t rlp_list_prefix_size(uint64_t content_len) {
  return content_len <= 55 ? 1 : 1 + rlp_length_width(content_len);
}

static uint8_t *write_be_length(uint8_t *out, uint64_t value,
                                unsigned width) {
  while (width-- != 0)
    *out++ = (uint8_t)(value >> (width * 8));
  return out;
}

uint8_t *rlp_write_string_prefix(uint8_t *out, uint64_t len,
                                         uint8_t first) {
  if (len == 1 && first < 0x80)
    return out;
  if (len <= 55) {
    *out++ = (uint8_t)(0x80 + len);
    return out;
  }
  const unsigned width = rlp_length_width(len);
  *out++ = (uint8_t)(0xb7 + width);
  return write_be_length(out, len, width);
}

uint8_t *rlp_write_list_prefix(uint8_t *out, uint64_t content_len) {
  if (content_len <= 55) {
    *out++ = (uint8_t)(0xc0 + content_len);
    return out;
  }
  const unsigned width = rlp_length_width(content_len);
  *out++ = (uint8_t)(0xf7 + width);
  return write_be_length(out, content_len, width);
}

uint8_t *rlp_write_u64(uint8_t *out, uint64_t value) {
  if (value == 0) {
    *out++ = 0x80;
    return out;
  }
  const unsigned width = be_u64_width(value);
  if (width == 1 && value < 0x80) {
    *out++ = (uint8_t)value;
    return out;
  }
  *out++ = (uint8_t)(0x80 + width);
  return write_be_length(out, value, width);
}

uint8_t *rlp_write_u256(uint8_t *out, U256 value) {
  uint8_t bytes[32];
  sail_word_to_be_bytes(bytes, value);
  const unsigned width = u256_width(value);
  if (width == 0) {
    *out++ = 0x80;
    return out;
  }
  const uint8_t *first = bytes + sizeof(bytes) - width;
  if (width == 1 && first[0] < 0x80) {
    *out++ = first[0];
    return out;
  }
  *out++ = (uint8_t)(0x80 + width);
  memcpy(out, first, width);
  return out + width;
}
