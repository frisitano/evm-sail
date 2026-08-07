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
#include "primitives/value.h"
#include "zkvm_accelerators.h"

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const uint8_t preimage_empty = 0;

bytes32 zero_hash(void)
{
  bytes32 result = {{0}};
  return result;
}

bytes32 keccak_bytes(const uint8_t *bytes, uint64_t len)
{
  zkvm_keccak256_hash native_digest;
  bytes32 digest = {{0}};
  _Static_assert(sizeof(digest) == sizeof(zkvm_keccak256_hash),
                 "accelerator digest must be 32 bytes");
  if (len <= UINT32_MAX &&
      zkvm_keccak256(len ? bytes : &preimage_empty, (size_t)len, &native_digest) == ZKVM_EOK) {
    memcpy(&digest, &native_digest, sizeof digest);
  }
  return digest;
}

bytes32 sha256_bytes(const uint8_t *bytes, uint64_t len)
{
  zkvm_sha256_hash native_digest;
  bytes32 digest = {{0}};
  _Static_assert(sizeof(digest) == sizeof(zkvm_sha256_hash), "accelerator digest must be 32 bytes");
  if (len <= UINT32_MAX &&
      zkvm_sha256(len ? bytes : &preimage_empty, (size_t)len, &native_digest) == ZKVM_EOK) {
    memcpy(&digest, &native_digest, sizeof digest);
  }
  return digest;
}

uint64_t rlp_length_byte_native(uint8_t value)
{
  return value;
}

static unsigned be_u64_width(uint64_t value)
{
  unsigned width = 0;
  while (value != 0) {
    ++width;
    value >>= 8;
  }
  return width;
}

static unsigned u256_width(u256 value)
{
  for (size_t limb = 4; limb != 0; --limb) {
    const uint64_t value_limb = value.limbs[limb - 1];
    if (value_limb != 0) {
      return (unsigned)((limb - 1) * sizeof(value_limb)) + be_u64_width(value_limb);
    }
  }
  return 0;
}

uint64_t rlp_quantity_size_u64(uint64_t value)
{
  if (value < 0x80) {
    return 1;
  }
  return 1 + be_u64_width(value);
}

uint64_t rlp_quantity_size_u256(u256 value)
{
  const unsigned width = u256_width(value);
  if (width == 0) {
    return 1;
  }
  if (width == 1 && value.limbs[0] < 0x80) {
    return 1;
  }
  return 1 + width;
}

uint64_t rlp_string_prefix_size(uint64_t len, uint8_t first)
{
  if (len == 1 && first < 0x80) {
    return 0;
  }
  if (len <= 55) {
    return 1;
  }
  return 1 + be_u64_width(len);
}

uint64_t rlp_string_size(uint64_t len, uint8_t first)
{
  return len + rlp_string_prefix_size(len, first);
}

uint64_t rlp_list_prefix_size(uint64_t content_len)
{
  return content_len <= 55 ? 1 : 1 + be_u64_width(content_len);
}

static uint8_t *write_be_length(uint8_t *out, uint64_t value, unsigned width)
{
  while (width-- != 0) {
    *out++ = (uint8_t)(value >> (width * 8));
  }
  return out;
}

uint8_t *rlp_write_string_prefix(uint8_t *out, uint64_t len, uint8_t first)
{
  if (len == 1 && first < 0x80) {
    return out;
  }
  if (len <= 55) {
    *out++ = (uint8_t)(0x80 + len);
    return out;
  }
  const unsigned width = be_u64_width(len);
  *out++ = (uint8_t)(0xb7 + width);
  return write_be_length(out, len, width);
}

uint8_t *rlp_write_list_prefix(uint8_t *out, uint64_t content_len)
{
  if (content_len <= 55) {
    *out++ = (uint8_t)(0xc0 + content_len);
    return out;
  }
  const unsigned width = be_u64_width(content_len);
  *out++ = (uint8_t)(0xf7 + width);
  return write_be_length(out, content_len, width);
}

uint8_t *rlp_write_u64(uint8_t *out, uint64_t value)
{
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

uint8_t *rlp_write_u256(uint8_t *out, u256 value)
{
  const unsigned width = u256_width(value);
  if (width == 0) {
    *out++ = 0x80;
    return out;
  }
  if (width == 1 && value.limbs[0] < 0x80) {
    *out++ = (uint8_t)value.limbs[0];
    return out;
  }
  *out++ = (uint8_t)(0x80 + width);

  const unsigned top_limb = (width - 1) / 8;
  const unsigned top_width = width - (top_limb * 8);
  out = write_be_length(out, value.limbs[top_limb], top_width);
  for (unsigned limb = top_limb; limb != 0; --limb) {
    const uint64_t big_endian_limb = bswap64(value.limbs[limb - 1]);
    memcpy(out, &big_endian_limb, sizeof(big_endian_limb));
    out += sizeof(big_endian_limb);
  }
  return out;
}
