#include "evmsail/host/accelerators.h"

#include "evmsail/host/output.h"
#include "evmsail/spec/prelude.h"
#include "evmsail/prelude.h"
#include "evmsail/spec/primitives/bytes.h"
#include "primitives/value.h"
#include "workspace.h"
#include "zkvm_accelerators.h"

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct {
  const uint8_t *bytes;
  uint32_t length;
} AcceleratorInput;

static const uint8_t empty_input[1] = {0};
static uint8_t *input_scratch;
static uint8_t *bls_scratch;

void accelerators_workspace_bind(void)
{
  input_scratch = (uint8_t *)workspace_claim(GUEST_ACCELERATOR_INPUT_BYTES, 1, _Alignof(uint64_t));
  bls_scratch = (uint8_t *)workspace_claim(GUEST_ACCELERATOR_WORK_BYTES, 1, _Alignof(uint64_t));
}

/* calldata_span accepts the canonical null empty slice and nominal calldata
 * regions are 32-bit-bounded, so no length or pointer re-check is needed here. */
static AcceleratorInput resolve_input(const uint8_t *bytes, uint64_t len)
{
  return (AcceleratorInput){len ? bytes : empty_input, (uint32_t)len};
}

static uint8_t input_byte(const AcceleratorInput *input, uint64_t index)
{
  return index < input->length ? input->bytes[index] : 0;
}

static void copy_padded(const AcceleratorInput *input, uint8_t *out, uint64_t off, uint64_t len)
{
  for (uint64_t i = 0; i < len; i++) {
    out[i] = input_byte(input, off + i);
  }
}

static uint8_t *materialize(const AcceleratorInput *input, uint64_t padded_len)
    __attribute__((assume_aligned(8)));

static uint8_t *materialize(const AcceleratorInput *input, uint64_t padded_len)
{
  if (padded_len > GUEST_ACCELERATOR_INPUT_BYTES) {
    return NULL;
  }
  uint8_t *bytes = input_scratch;
  uint64_t copied = input->length < padded_len ? input->length : padded_len;
  if (copied) {
    memmove(bytes, input->bytes, (size_t)copied);
  }
  if (padded_len > copied) {
    memset(bytes + copied, 0, (size_t)(padded_len - copied));
  }
  return bytes;
}

static bool output_failed(void)
{
  (void)output_buffer_finish(0);
  return false;
}

static bool accelerator_ripemd160_bytes(const uint8_t *bytes, uint64_t len)
{
  const AcceleratorInput input = resolve_input(bytes, len);
  zkvm_ripemd160_hash digest;
  if (zkvm_ripemd160(input.bytes, input.length, &digest) != ZKVM_EOK) {
    return output_failed();
  }
  uint8_t *out = output_buffer_reserve(32);
  if (!out) {
    return output_failed();
  }
  memcpy(out, digest.data, 32);
  return output_buffer_finish(32);
}

/* Field lengths are Sail calldata_length values (32-bit-bounded), so the
 * padded-length sum cannot overflow. */
static bool accelerator_modexp_bytes(const uint8_t *bytes, uint64_t len, uint64_t base_len,
                                     uint64_t exponent_len, uint64_t modulus_len)
{
  const AcceleratorInput input = resolve_input(bytes, len);
  uint64_t padded_len = 96 + base_len + exponent_len + modulus_len;
  uint8_t *materialized = materialize(&input, padded_len);
  uint8_t *out = output_buffer_reserve(modulus_len);
  if (!materialized || !out) {
    return output_failed();
  }
  if (zkvm_modexp(materialized + 96, (size_t)base_len, materialized + 96 + base_len,
                  (size_t)exponent_len, materialized + 96 + base_len + exponent_len,
                  (size_t)modulus_len, out) != ZKVM_EOK) {
    return output_failed();
  }
  return output_buffer_finish(modulus_len);
}

static bool accelerator_bn254_add_bytes(const uint8_t *bytes, uint64_t len)
{
  const AcceleratorInput input = resolve_input(bytes, len);
  _Alignas(uint64_t) uint8_t encoded[128];
  _Alignas(uint64_t) uint8_t result[64];
  copy_padded(&input, encoded, 0, sizeof encoded);
  uint8_t *out = output_buffer_reserve(64);
  if (!out) {
    return output_failed();
  }
  if (zkvm_bn254_g1_add((const zkvm_bn254_g1_point *)encoded,
                        (const zkvm_bn254_g1_point *)(encoded + 64),
                        (zkvm_bn254_g1_point *)result) != ZKVM_EOK) {
    return output_failed();
  }
  memcpy(out, result, sizeof result);
  return output_buffer_finish(64);
}

static bool accelerator_bn254_mul_bytes(const uint8_t *bytes, uint64_t len)
{
  const AcceleratorInput input = resolve_input(bytes, len);
  _Alignas(uint64_t) uint8_t encoded[96];
  _Alignas(uint64_t) uint8_t result[64];
  copy_padded(&input, encoded, 0, sizeof encoded);
  uint8_t *out = output_buffer_reserve(64);
  if (!out) {
    return output_failed();
  }
  if (zkvm_bn254_g1_mul((const zkvm_bn254_g1_point *)encoded,
                        (const zkvm_bn254_scalar *)(encoded + 64),
                        (zkvm_bn254_g1_point *)result) != ZKVM_EOK) {
    return output_failed();
  }
  memcpy(out, result, sizeof result);
  return output_buffer_finish(64);
}

static uint8_t accelerator_bn254_pairing_bytes(const uint8_t *bytes, uint64_t len)
{
  bool verified = false;
  if (len % 192 != 0) {
    return 0;
  }
  const AcceleratorInput input = resolve_input(bytes, len);
  uint8_t *encoded = materialize(&input, len);
  if (encoded == NULL) {
    return 0;
  }
  const zkvm_bn254_pairing_pair *pairs = (const zkvm_bn254_pairing_pair *)__builtin_assume_aligned(
      encoded, _Alignof(zkvm_bn254_pairing_pair));
  if (zkvm_bn254_pairing(pairs, (size_t)(len / 192), &verified) != ZKVM_EOK) {
    return 0;
  }
  return (int)verified ? UINT8_C(3) : UINT8_C(2);
}

/* Sail: rounds is blake2_rounds (32-bit-bounded) and final_block is
 * y_parity = range(0, 1) (sail/host/accelerators.sail). */
static bool accelerator_blake2f_bytes(const uint8_t *bytes, uint64_t len, uint64_t rounds,
                                      uint64_t final_block)
{
  _Alignas(uint64_t) uint8_t blake_state[64];
  _Alignas(uint64_t) uint8_t blake_message[128];
  _Alignas(uint64_t) uint8_t offset_counter[16];
  if (len != 213) {
    return output_failed();
  }
  const AcceleratorInput input = resolve_input(bytes, len);
  copy_padded(&input, blake_state, 4, sizeof blake_state);
  copy_padded(&input, blake_message, 68, sizeof blake_message);
  copy_padded(&input, offset_counter, 196, sizeof offset_counter);
  uint8_t *out = output_buffer_reserve(64);
  if (!out) {
    return output_failed();
  }
  if (zkvm_blake2f((uint32_t)rounds, (zkvm_blake2f_state *)blake_state,
                   (const zkvm_blake2f_message *)blake_message,
                   (const zkvm_blake2f_offset *)offset_counter, (uint8_t)final_block) != ZKVM_EOK) {
    return output_failed();
  }
  memcpy(out, blake_state, sizeof blake_state);
  return output_buffer_finish(64);
}

static bool accelerator_kzg_point_evaluation_bytes(const uint8_t *bytes, uint64_t len)
{
  _Alignas(uint64_t) uint8_t commitment[48];
  _Alignas(uint64_t) uint8_t z[32];
  _Alignas(uint64_t) uint8_t y[32];
  _Alignas(uint64_t) uint8_t proof[48];
  bool verified = false;
  if (len != 192) {
    return false;
  }
  const AcceleratorInput input = resolve_input(bytes, len);
  copy_padded(&input, z, 32, sizeof z);
  copy_padded(&input, y, 64, sizeof y);
  copy_padded(&input, commitment, 96, sizeof commitment);
  copy_padded(&input, proof, 144, sizeof proof);
  return (zkvm_kzg_point_eval((const zkvm_kzg_commitment *)commitment,
                              (const zkvm_kzg_field_element *)z, (const zkvm_kzg_field_element *)y,
                              (const zkvm_kzg_proof *)proof, &verified) == ZKVM_EOK &&
          verified) != 0;
}

/* EIP-2537 uses 64-byte-padded Fp values; the accelerator ABI uses compact
 * 48-byte values. Sail validates the zero padding before these adapters run. */
static void compact_fp(const AcceleratorInput *input, uint8_t *out, uint64_t off)
{
  copy_padded(input, out, off + 16, 48);
}

static void pad_fp(uint8_t *out, uint64_t off, const uint8_t *compact)
{
  memset(out + off, 0, 16);
  memcpy(out + off + 16, compact, 48);
}

static void compact_g1(const AcceleratorInput *input, uint8_t *out, uint64_t off)
{
  compact_fp(input, out, off);
  compact_fp(input, out + 48, off + 64);
}

static void pad_g1(uint8_t *out, const uint8_t *compact)
{
  pad_fp(out, 0, compact);
  pad_fp(out, 64, compact + 48);
}

/* blst serializes Fp2 components c1||c0 while the EVM layout is c0||c1. */
static void compact_g2(const AcceleratorInput *input, uint8_t *out, uint64_t off)
{
  compact_fp(input, out + 48, off);
  compact_fp(input, out, off + 64);
  compact_fp(input, out + 144, off + 128);
  compact_fp(input, out + 96, off + 192);
}

static void pad_g2(uint8_t *out, const uint8_t *compact)
{
  pad_fp(out, 0, compact + 48);
  pad_fp(out, 64, compact);
  pad_fp(out, 128, compact + 144);
  pad_fp(out, 192, compact + 96);
}

static uint8_t *reserve_bls(uint64_t count, uint64_t item_size) __attribute__((assume_aligned(8)));

static uint8_t *reserve_bls(uint64_t count, uint64_t item_size)
{
  if (count && item_size > UINT64_MAX / count) {
    return NULL;
  }
  return count * item_size <= GUEST_ACCELERATOR_WORK_BYTES ? bls_scratch : NULL;
}

static bool accelerator_bls_g1_add_bytes(const uint8_t *bytes, uint64_t len)
{
  _Alignas(uint64_t) uint8_t encoded[192];
  _Alignas(uint64_t) uint8_t compact_result[96];
  if (len != 256) {
    return output_failed();
  }
  const AcceleratorInput input = resolve_input(bytes, len);
  compact_g1(&input, encoded, 0);
  compact_g1(&input, encoded + 96, 128);
  uint8_t *out = output_buffer_reserve(128);
  if (!out) {
    return output_failed();
  }
  if (zkvm_bls12_g1_add((const zkvm_bls12_381_g1_point *)encoded,
                        (const zkvm_bls12_381_g1_point *)(encoded + 96),
                        (zkvm_bls12_381_g1_point *)compact_result) != ZKVM_EOK) {
    return output_failed();
  }
  pad_g1(out, compact_result);
  return output_buffer_finish(128);
}

static bool accelerator_bls_g1_msm_bytes(const uint8_t *bytes, uint64_t len)
{
  _Alignas(uint64_t) uint8_t compact_result[96];
  if (len == 0 || len % 160 != 0) {
    return output_failed();
  }
  const AcceleratorInput input = resolve_input(bytes, len);
  uint64_t count = len / 160;
  uint8_t *pairs = reserve_bls(count, 128);
  if (!pairs) {
    return output_failed();
  }
  for (uint64_t i = 0; i < count; i++) {
    compact_g1(&input, pairs + (i * 128), i * 160);
    copy_padded(&input, pairs + (i * 128) + 96, (i * 160) + 128, 32);
  }
  uint8_t *out = output_buffer_reserve(128);
  if (!out) {
    return output_failed();
  }
  const zkvm_bls12_381_g1_msm_pair *typed_pairs =
      (const zkvm_bls12_381_g1_msm_pair *)__builtin_assume_aligned(
          pairs, _Alignof(zkvm_bls12_381_g1_msm_pair));
  if (zkvm_bls12_g1_msm(typed_pairs, (size_t)count, (zkvm_bls12_381_g1_point *)compact_result) !=
      ZKVM_EOK) {
    return output_failed();
  }
  pad_g1(out, compact_result);
  return output_buffer_finish(128);
}

static bool accelerator_bls_g2_add_bytes(const uint8_t *bytes, uint64_t len)
{
  _Alignas(uint64_t) uint8_t encoded[384];
  _Alignas(uint64_t) uint8_t compact_result[192];
  if (len != 512) {
    return output_failed();
  }
  const AcceleratorInput input = resolve_input(bytes, len);
  compact_g2(&input, encoded, 0);
  compact_g2(&input, encoded + 192, 256);
  uint8_t *out = output_buffer_reserve(256);
  if (!out) {
    return output_failed();
  }
  if (zkvm_bls12_g2_add((const zkvm_bls12_381_g2_point *)encoded,
                        (const zkvm_bls12_381_g2_point *)(encoded + 192),
                        (zkvm_bls12_381_g2_point *)compact_result) != ZKVM_EOK) {
    return output_failed();
  }
  pad_g2(out, compact_result);
  return output_buffer_finish(256);
}

static bool accelerator_bls_g2_msm_bytes(const uint8_t *bytes, uint64_t len)
{
  _Alignas(uint64_t) uint8_t compact_result[192];
  if (len == 0 || len % 288 != 0) {
    return output_failed();
  }
  const AcceleratorInput input = resolve_input(bytes, len);
  uint64_t count = len / 288;
  uint8_t *pairs = reserve_bls(count, 224);
  if (!pairs) {
    return output_failed();
  }
  for (uint64_t i = 0; i < count; i++) {
    compact_g2(&input, pairs + (i * 224), i * 288);
    copy_padded(&input, pairs + (i * 224) + 192, (i * 288) + 256, 32);
  }
  uint8_t *out = output_buffer_reserve(256);
  if (!out) {
    return output_failed();
  }
  const zkvm_bls12_381_g2_msm_pair *typed_pairs =
      (const zkvm_bls12_381_g2_msm_pair *)__builtin_assume_aligned(
          pairs, _Alignof(zkvm_bls12_381_g2_msm_pair));
  if (zkvm_bls12_g2_msm(typed_pairs, (size_t)count, (zkvm_bls12_381_g2_point *)compact_result) !=
      ZKVM_EOK) {
    return output_failed();
  }
  pad_g2(out, compact_result);
  return output_buffer_finish(256);
}

static uint8_t accelerator_bls_pairing_bytes(const uint8_t *bytes, uint64_t len)
{
  bool verified = false;
  if (len == 0 || len % 384 != 0) {
    return 0;
  }
  const AcceleratorInput input = resolve_input(bytes, len);
  uint64_t count = len / 384;
  uint8_t *pairs = reserve_bls(count, 288);
  if (!pairs) {
    return 0;
  }
  for (uint64_t i = 0; i < count; i++) {
    compact_g1(&input, pairs + (i * 288), i * 384);
    compact_g2(&input, pairs + (i * 288) + 96, (i * 384) + 128);
  }
  const zkvm_bls12_381_pairing_pair *typed_pairs =
      (const zkvm_bls12_381_pairing_pair *)__builtin_assume_aligned(
          pairs, _Alignof(zkvm_bls12_381_pairing_pair));
  if (zkvm_bls12_pairing(typed_pairs, (size_t)count, &verified) != ZKVM_EOK) {
    return 0;
  }
  return (int)verified ? UINT8_C(3) : UINT8_C(2);
}

static bool accelerator_bls_map_fp_to_g1_bytes(const uint8_t *bytes, uint64_t len)
{
  _Alignas(uint64_t) uint8_t field[48];
  _Alignas(uint64_t) uint8_t compact_result[96];
  if (len != 64) {
    return output_failed();
  }
  const AcceleratorInput input = resolve_input(bytes, len);
  compact_fp(&input, field, 0);
  uint8_t *out = output_buffer_reserve(128);
  if (!out) {
    return output_failed();
  }
  if (zkvm_bls12_map_fp_to_g1((const zkvm_bls12_381_fp *)field,
                              (zkvm_bls12_381_g1_point *)compact_result) != ZKVM_EOK) {
    return output_failed();
  }
  pad_g1(out, compact_result);
  return output_buffer_finish(128);
}

static bool accelerator_bls_map_fp2_to_g2_bytes(const uint8_t *bytes, uint64_t len)
{
  _Alignas(uint64_t) uint8_t field[96];
  _Alignas(uint64_t) uint8_t compact_result[192];
  if (len != 128) {
    return output_failed();
  }
  const AcceleratorInput input = resolve_input(bytes, len);
  compact_fp(&input, field, 0);
  compact_fp(&input, field + 48, 64);
  uint8_t *out = output_buffer_reserve(256);
  if (!out) {
    return output_failed();
  }
  if (zkvm_bls12_map_fp2_to_g2((const zkvm_bls12_381_fp2 *)field,
                               (zkvm_bls12_381_g2_point *)compact_result) != ZKVM_EOK) {
    return output_failed();
  }
  pad_g2(out, compact_result);
  return output_buffer_finish(256);
}

static bool accelerator_p256_verify_bytes(const uint8_t *bytes, uint64_t len)
{
  bool verified = false;
  if (len != 160) {
    return false;
  }
  const AcceleratorInput input = resolve_input(bytes, len);
  uint8_t *encoded = materialize(&input, 160);
  if (encoded == NULL) {
    return false;
  }
  const zkvm_secp256r1_hash *hash =
      (const zkvm_secp256r1_hash *)__builtin_assume_aligned(encoded, _Alignof(zkvm_secp256r1_hash));
  const zkvm_secp256r1_signature *signature =
      (const zkvm_secp256r1_signature *)__builtin_assume_aligned(
          encoded + 32, _Alignof(zkvm_secp256r1_signature));
  const zkvm_secp256r1_pubkey *public_key = (const zkvm_secp256r1_pubkey *)__builtin_assume_aligned(
      encoded + 96, _Alignof(zkvm_secp256r1_pubkey));
  return (zkvm_secp256r1_verify(hash, signature, public_key, &verified) == ZKVM_EOK && verified) !=
         0;
}

/* Sail: yparity is y_parity = range(0, 1)
 * (sail/host/accelerators.sail, sail/primitives/quantities.sail). */
struct AddressResult precompile_ecrecover_hash_sig(Hash32 hash, uint8_t yparity, U256 r, U256 s)
{
  struct AddressResult result = {0};
  uint8_t address[20] = {0};
  zkvm_secp256k1_signature signature;
  zkvm_secp256k1_hash hash_value;
  zkvm_secp256k1_pubkey public_key;
  zkvm_keccak256_hash address_hash = {{0}};
  memset(&public_key, 0, sizeof public_key);
  sail_word_to_be_bytes(signature.data, r);
  sail_word_to_be_bytes(signature.data + 32, s);
  memcpy(hash_value.data, hash_bytes_const(&hash), sizeof hash_value.data);
  bool ok = zkvm_secp256k1_ecrecover(&hash_value, &signature, yparity, &public_key) == ZKVM_EOK;
  if (ok) {
    ok = zkvm_keccak256(public_key.data, sizeof public_key.data, &address_hash) == ZKVM_EOK;
  }
  if (ok) {
    memcpy(address, address_hash.data + 12, 20);
  }
  result.success = ok;
  result.address = address_from_be_bytes(address);
  return result;
}

/* ------------------- Sail-facing calldata dispatch ---------------------- */

static bool calldata_span(struct CalldataSlice input, const uint8_t **bytes, uint64_t *len)
{
  switch (input.kind) {
  case Kind_InputCalldata:
    *len = input.variants.InputCalldata.len;
    *bytes = input.variants.InputCalldata.bytes;
    return (*len == 0 || *bytes != NULL) != 0;
  case Kind_MemoryCalldata:
    *len = input.variants.MemoryCalldata.len;
    *bytes = input.variants.MemoryCalldata.bytes;
    return (*len == 0 || *bytes != NULL) != 0;
  }
  return false;
}

#define CALL_ACCELERATOR(input, call)                                                              \
  do {                                                                                             \
    const uint8_t *bytes = NULL;                                                                   \
    uint64_t len = 0;                                                                              \
    if (!calldata_span((input), &bytes, &len))                                                     \
      return false;                                                                                \
    return call;                                                                                   \
  } while (0)

bool accelerator_ripemd160(struct CalldataSlice input)
{
  CALL_ACCELERATOR(input, accelerator_ripemd160_bytes(bytes, len));
}

bool accelerator_modexp(struct CalldataSlice input, uint32_t base_len, uint32_t exponent_len,
                        uint32_t modulus_len)
{
  CALL_ACCELERATOR(input,
                   accelerator_modexp_bytes(bytes, len, base_len, exponent_len, modulus_len));
}

bool accelerator_bn254_add(struct CalldataSlice input)
{
  CALL_ACCELERATOR(input, accelerator_bn254_add_bytes(bytes, len));
}

bool accelerator_bn254_mul(struct CalldataSlice input)
{
  CALL_ACCELERATOR(input, accelerator_bn254_mul_bytes(bytes, len));
}

uint8_t accelerator_bn254_pairing(struct CalldataSlice input)
{
  const uint8_t *bytes = NULL;
  uint64_t len = 0;
  if (!calldata_span(input, &bytes, &len)) {
    return 0;
  }
  return accelerator_bn254_pairing_bytes(bytes, len);
}

bool accelerator_blake2f(struct CalldataSlice input, uint32_t rounds, uint8_t final_block)
{
  CALL_ACCELERATOR(input, accelerator_blake2f_bytes(bytes, len, rounds, final_block));
}

bool accelerator_kzg_point_evaluation(struct CalldataSlice input)
{
  CALL_ACCELERATOR(input, accelerator_kzg_point_evaluation_bytes(bytes, len));
}

bool accelerator_bls_g1_add(struct CalldataSlice input)
{
  CALL_ACCELERATOR(input, accelerator_bls_g1_add_bytes(bytes, len));
}

bool accelerator_bls_g1_msm(struct CalldataSlice input)
{
  CALL_ACCELERATOR(input, accelerator_bls_g1_msm_bytes(bytes, len));
}

bool accelerator_bls_g2_add(struct CalldataSlice input)
{
  CALL_ACCELERATOR(input, accelerator_bls_g2_add_bytes(bytes, len));
}

bool accelerator_bls_g2_msm(struct CalldataSlice input)
{
  CALL_ACCELERATOR(input, accelerator_bls_g2_msm_bytes(bytes, len));
}

uint8_t accelerator_bls_pairing(struct CalldataSlice input)
{
  const uint8_t *bytes = NULL;
  uint64_t len = 0;
  if (!calldata_span(input, &bytes, &len)) {
    return 0;
  }
  return accelerator_bls_pairing_bytes(bytes, len);
}

bool accelerator_bls_map_fp_to_g1(struct CalldataSlice input)
{
  CALL_ACCELERATOR(input, accelerator_bls_map_fp_to_g1_bytes(bytes, len));
}

bool accelerator_bls_map_fp2_to_g2(struct CalldataSlice input)
{
  CALL_ACCELERATOR(input, accelerator_bls_map_fp2_to_g2_bytes(bytes, len));
}

bool accelerator_p256_verify(struct CalldataSlice input)
{
  CALL_ACCELERATOR(input, accelerator_p256_verify_bytes(bytes, len));
}

#undef CALL_ACCELERATOR
