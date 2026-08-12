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

static bytes32 crypto_keccak_bytes(const uint8_t *bytes, uint64_t len)
{
  zkvm_keccak256_hash native_digest = {{0}};
  _Static_assert(sizeof(bytes32) == sizeof(native_digest), "accelerator digest must be 32 bytes");
  if ((!bytes && len != 0) || len > SIZE_MAX ||
      zkvm_keccak256(len ? bytes : &hash_empty, (size_t)len, &native_digest) != ZKVM_EOK) {
    memset(&native_digest, 0, sizeof(native_digest));
  }
  return hash_from_be_bytes(native_digest.data);
}

static bytes32 crypto_sha256_bytes(const uint8_t *bytes, uint64_t len)
{
  zkvm_sha256_hash native_digest = {{0}};
  _Static_assert(sizeof(bytes32) == sizeof(native_digest), "accelerator digest must be 32 bytes");
  if ((!bytes && len != 0) || len > SIZE_MAX ||
      zkvm_sha256(len ? bytes : &hash_empty, (size_t)len, &native_digest) != ZKVM_EOK) {
    memset(&native_digest, 0, sizeof(native_digest));
  }
  return hash_from_be_bytes(native_digest.data);
}

bytes32 host_keccak_stateless_input(Bytes input)
{
  return crypto_keccak_bytes(input.bytes, input.len);
}

bytes32 host_keccak_scratch(Bytes input)
{
  return crypto_keccak_bytes(input.bytes, input.len);
}

bytes32 host_keccak_memory(Bytes input)
{
  return crypto_keccak_bytes(input.bytes, input.len);
}

bytes32 host_keccak_output(Bytes input)
{
  return crypto_keccak_bytes(input.bytes, input.len);
}

bytes32 host_keccak_log_data(Bytes input)
{
  return crypto_keccak_bytes(input.bytes, input.len);
}

bytes32 host_sha256_stateless_input(Bytes input)
{
  return crypto_sha256_bytes(input.bytes, input.len);
}

bytes32 host_sha256_scratch(Bytes input)
{
  return crypto_sha256_bytes(input.bytes, input.len);
}

bytes32 host_sha256_memory(Bytes input)
{
  return crypto_sha256_bytes(input.bytes, input.len);
}

bytes32 host_keccak_code(Bytes input)
{
  return crypto_keccak_bytes(input.bytes, input.len);
}

bytes32 host_keccak_word(const u256 input)
{
  uint8_t bytes[32];
  zkvm_keccak256_hash native_digest = {{0}};
  _Static_assert(sizeof(bytes32) == sizeof(zkvm_keccak256_hash),
                 "accelerator digest must be 32 bytes");
  sail_word_to_be_bytes(bytes, input);
  if (zkvm_keccak256(bytes, sizeof(bytes), &native_digest) != ZKVM_EOK) {
    memset(&native_digest, 0, sizeof native_digest);
  }
  return hash_from_be_bytes(native_digest.data);
}

bytes32 host_keccak_address(bytes20 input)
{
  zkvm_keccak256_hash native_digest = {{0}};
  _Static_assert(sizeof(bytes32) == sizeof(zkvm_keccak256_hash),
                 "accelerator digest must be 32 bytes");
  if (zkvm_keccak256(bytes20_data(&input), 20, &native_digest) != ZKVM_EOK) {
    memset(&native_digest, 0, sizeof native_digest);
  }
  return hash_from_be_bytes(native_digest.data);
}

bytes32 host_sha256_pair(bytes32 left, bytes32 right)
{
  uint8_t bytes[64];
  zkvm_sha256_hash native_digest = {{0}};
  _Static_assert(sizeof(bytes32) == sizeof(zkvm_sha256_hash),
                 "accelerator digest must be 32 bytes");
  memcpy(bytes, bytes32_data(&left), 32);
  memcpy(bytes + 32, bytes32_data(&right), 32);
  if (zkvm_sha256(bytes, sizeof(bytes), &native_digest) != ZKVM_EOK) {
    memset(&native_digest, 0, sizeof native_digest);
  }
  return hash_from_be_bytes(native_digest.data);
}
