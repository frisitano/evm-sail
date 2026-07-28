#include "precompiles.h"

#include "value_convert.h"
#include "output.h"
#include "zkvm_accelerators.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ACCELERATOR_INPUT_MAX (1u << 21)

typedef struct {
  const uint8_t *bytes;
  uint32_t length;
} AcceleratorInput;

static const uint8_t empty_input[1] = {0};
static uint8_t *input_scratch;
static uint32_t input_scratch_capacity;
static uint8_t *bls_scratch;
static uint32_t bls_scratch_capacity;

static uint8_t *grow(uint8_t **buffer, uint32_t *capacity, uint64_t need) {
  if (need > ACCELERATOR_INPUT_MAX) return NULL;
  if (*buffer && *capacity >= need) return *buffer;
  uint32_t next = *capacity ? *capacity : 1024;
  while (next < need) next *= 2;
  uint8_t *bytes = (uint8_t *)realloc(*buffer, next);
  if (!bytes) return NULL;
  *buffer = bytes;
  *capacity = next;
  return bytes;
}

static bool resolve_input(AcceleratorInput *input, const uint8_t *bytes,
                          uint64_t len) {
  if (len > UINT32_MAX || (len && !bytes)) return false;
  input->bytes = len ? bytes : empty_input;
  input->length = (uint32_t)len;
  return true;
}

static uint8_t input_byte(const AcceleratorInput *input, uint64_t index) {
  return index < input->length ? input->bytes[index] : 0;
}

static void copy_padded(const AcceleratorInput *input, uint8_t *out,
                        uint64_t off, uint64_t len) {
  for (uint64_t i = 0; i < len; i++) out[i] = input_byte(input, off + i);
}

static uint8_t *materialize(const AcceleratorInput *input, uint64_t padded_len) {
  uint8_t *bytes = grow(&input_scratch, &input_scratch_capacity,
                        padded_len ? padded_len : 1);
  if (!bytes) return NULL;
  uint64_t copied = input->length < padded_len ? input->length : padded_len;
  if (copied) memmove(bytes, input->bytes, (size_t)copied);
  if (padded_len > copied)
    memset(bytes + copied, 0, (size_t)(padded_len - copied));
  return bytes;
}

static bool output_failed(void) {
  (void)output_buffer_finish(0);
  return false;
}

static uint8_t *reserve_output(uint64_t len) {
  return output_buffer_reserve(len);
}

static bool finish_output(uint64_t len) {
  return output_buffer_finish(len);
}

bool accelerator_ripemd160_bytes(const uint8_t *bytes, uint64_t len) {
  AcceleratorInput input;
  zkvm_ripemd160_hash digest;
  if (!resolve_input(&input, bytes, len) ||
      zkvm_ripemd160(input.bytes, input.length, &digest) != ZKVM_EOK)
    return output_failed();
  uint8_t *out = reserve_output(32);
  if (!out) return output_failed();
  memcpy(out, digest.data, 32);
  return finish_output(32);
}

bool accelerator_modexp_bytes(const uint8_t *bytes, uint64_t len,
                              uint64_t base_len, uint64_t exponent_len,
                              uint64_t modulus_len) {
  AcceleratorInput input;
  if (!resolve_input(&input, bytes, len)) return output_failed();
  if (base_len > UINT32_MAX || exponent_len > UINT32_MAX ||
      modulus_len > UINT32_MAX || base_len > UINT64_MAX - 96 ||
      exponent_len > UINT64_MAX - 96 - base_len ||
      modulus_len > UINT64_MAX - 96 - base_len - exponent_len)
    return output_failed();
  uint64_t padded_len = 96 + base_len + exponent_len + modulus_len;
  uint8_t *materialized = materialize(&input, padded_len);
  uint8_t *out = reserve_output(modulus_len);
  if (!materialized || !out) return output_failed();
  if (zkvm_modexp(materialized + 96, (size_t)base_len,
                  materialized + 96 + base_len, (size_t)exponent_len,
                  materialized + 96 + base_len + exponent_len,
                  (size_t)modulus_len, out) != ZKVM_EOK)
    return output_failed();
  return finish_output(modulus_len);
}

bool accelerator_bn254_add_bytes(const uint8_t *bytes, uint64_t len) {
  AcceleratorInput input;
  uint8_t encoded[128];
  if (!resolve_input(&input, bytes, len)) return output_failed();
  copy_padded(&input, encoded, 0, sizeof encoded);
  uint8_t *out = reserve_output(64);
  if (!out) return output_failed();
  if (zkvm_bn254_g1_add((const zkvm_bn254_g1_point *)encoded,
                        (const zkvm_bn254_g1_point *)(encoded + 64),
                        (zkvm_bn254_g1_point *)out) != ZKVM_EOK)
    return output_failed();
  return finish_output(64);
}

bool accelerator_bn254_mul_bytes(const uint8_t *bytes, uint64_t len) {
  AcceleratorInput input;
  uint8_t encoded[96];
  if (!resolve_input(&input, bytes, len)) return output_failed();
  copy_padded(&input, encoded, 0, sizeof encoded);
  uint8_t *out = reserve_output(64);
  if (!out) return output_failed();
  if (zkvm_bn254_g1_mul((const zkvm_bn254_g1_point *)encoded,
                        (const zkvm_bn254_scalar *)(encoded + 64),
                        (zkvm_bn254_g1_point *)out) != ZKVM_EOK)
    return output_failed();
  return finish_output(64);
}

uint64_t accelerator_bn254_pairing_bytes(const uint8_t *bytes, uint64_t len) {
  AcceleratorInput input;
  bool verified = false;
  if (len % 192 != 0 || !resolve_input(&input, bytes, len)) return 0;
  uint8_t *encoded = materialize(&input, len);
  if (!encoded ||
      zkvm_bn254_pairing((const zkvm_bn254_pairing_pair *)encoded,
                         (size_t)(len / 192), &verified) != ZKVM_EOK)
    return 0;
  return 2 | (verified ? 1 : 0);
}

bool accelerator_blake2f_bytes(const uint8_t *bytes, uint64_t len,
                               uint64_t rounds, uint64_t final_block) {
  AcceleratorInput input;
  uint8_t state[64], message[128], offset_counter[16];
  if (len != 213 || rounds > UINT32_MAX || final_block > 1 ||
      !resolve_input(&input, bytes, len))
    return output_failed();
  copy_padded(&input, state, 4, sizeof state);
  copy_padded(&input, message, 68, sizeof message);
  copy_padded(&input, offset_counter, 196, sizeof offset_counter);
  uint8_t *out = reserve_output(64);
  if (!out) return output_failed();
  if (zkvm_blake2f((uint32_t)rounds, (zkvm_blake2f_state *)state,
                   (const zkvm_blake2f_message *)message,
                   (const zkvm_blake2f_offset *)offset_counter,
                   (uint8_t)final_block) != ZKVM_EOK)
    return output_failed();
  memcpy(out, state, sizeof state);
  return finish_output(64);
}

bool accelerator_kzg_point_evaluation_bytes(const uint8_t *bytes,
                                            uint64_t len) {
  AcceleratorInput input;
  uint8_t commitment[48], z[32], y[32], proof[48];
  bool verified = false;
  if (len != 192 || !resolve_input(&input, bytes, len)) return false;
  copy_padded(&input, z, 32, sizeof z);
  copy_padded(&input, y, 64, sizeof y);
  copy_padded(&input, commitment, 96, sizeof commitment);
  copy_padded(&input, proof, 144, sizeof proof);
  return zkvm_kzg_point_eval((const zkvm_kzg_commitment *)commitment,
                             (const zkvm_kzg_field_element *)z,
                             (const zkvm_kzg_field_element *)y,
                             (const zkvm_kzg_proof *)proof,
                             &verified) == ZKVM_EOK && verified;
}

/* EIP-2537 uses 64-byte-padded Fp values; the accelerator ABI uses compact
 * 48-byte values. Sail validates the zero padding before these adapters run. */
static void compact_fp(const AcceleratorInput *input, uint8_t *out,
                       uint64_t off) {
  copy_padded(input, out, off + 16, 48);
}

static void pad_fp(uint8_t *out, uint64_t off, const uint8_t *compact) {
  memset(out + off, 0, 16);
  memcpy(out + off + 16, compact, 48);
}

static void compact_g1(const AcceleratorInput *input, uint8_t *out,
                       uint64_t off) {
  compact_fp(input, out, off);
  compact_fp(input, out + 48, off + 64);
}

static void pad_g1(uint8_t *out, const uint8_t *compact) {
  pad_fp(out, 0, compact);
  pad_fp(out, 64, compact + 48);
}

/* blst serializes Fp2 components c1||c0 while the EVM layout is c0||c1. */
static void compact_g2(const AcceleratorInput *input, uint8_t *out,
                       uint64_t off) {
  compact_fp(input, out + 48, off);
  compact_fp(input, out, off + 64);
  compact_fp(input, out + 144, off + 128);
  compact_fp(input, out + 96, off + 192);
}

static void pad_g2(uint8_t *out, const uint8_t *compact) {
  pad_fp(out, 0, compact + 48);
  pad_fp(out, 64, compact);
  pad_fp(out, 128, compact + 144);
  pad_fp(out, 192, compact + 96);
}

static uint8_t *reserve_bls(uint64_t count, uint64_t item_size) {
  if (count && item_size > UINT64_MAX / count) return NULL;
  return grow(&bls_scratch, &bls_scratch_capacity, count * item_size);
}

bool accelerator_bls_g1_add_bytes(const uint8_t *bytes, uint64_t len) {
  AcceleratorInput input;
  uint8_t encoded[192], compact_result[96];
  if (len != 256 || !resolve_input(&input, bytes, len))
    return output_failed();
  compact_g1(&input, encoded, 0);
  compact_g1(&input, encoded + 96, 128);
  uint8_t *out = reserve_output(128);
  if (!out) return output_failed();
  if (zkvm_bls12_g1_add((const zkvm_bls12_381_g1_point *)encoded,
                        (const zkvm_bls12_381_g1_point *)(encoded + 96),
                        (zkvm_bls12_381_g1_point *)compact_result) != ZKVM_EOK)
    return output_failed();
  pad_g1(out, compact_result);
  return finish_output(128);
}

bool accelerator_bls_g1_msm_bytes(const uint8_t *bytes, uint64_t len) {
  AcceleratorInput input;
  uint8_t compact_result[96];
  if (len == 0 || len % 160 != 0 || !resolve_input(&input, bytes, len))
    return output_failed();
  uint64_t count = len / 160;
  uint8_t *pairs = reserve_bls(count, 128);
  if (!pairs) return output_failed();
  for (uint64_t i = 0; i < count; i++) {
    compact_g1(&input, pairs + i * 128, i * 160);
    copy_padded(&input, pairs + i * 128 + 96, i * 160 + 128, 32);
  }
  uint8_t *out = reserve_output(128);
  if (!out) return output_failed();
  if (zkvm_bls12_g1_msm((const zkvm_bls12_381_g1_msm_pair *)pairs,
                        (size_t)count,
                        (zkvm_bls12_381_g1_point *)compact_result) != ZKVM_EOK)
    return output_failed();
  pad_g1(out, compact_result);
  return finish_output(128);
}

bool accelerator_bls_g2_add_bytes(const uint8_t *bytes, uint64_t len) {
  AcceleratorInput input;
  uint8_t encoded[384], compact_result[192];
  if (len != 512 || !resolve_input(&input, bytes, len))
    return output_failed();
  compact_g2(&input, encoded, 0);
  compact_g2(&input, encoded + 192, 256);
  uint8_t *out = reserve_output(256);
  if (!out) return output_failed();
  if (zkvm_bls12_g2_add((const zkvm_bls12_381_g2_point *)encoded,
                        (const zkvm_bls12_381_g2_point *)(encoded + 192),
                        (zkvm_bls12_381_g2_point *)compact_result) != ZKVM_EOK)
    return output_failed();
  pad_g2(out, compact_result);
  return finish_output(256);
}

bool accelerator_bls_g2_msm_bytes(const uint8_t *bytes, uint64_t len) {
  AcceleratorInput input;
  uint8_t compact_result[192];
  if (len == 0 || len % 288 != 0 || !resolve_input(&input, bytes, len))
    return output_failed();
  uint64_t count = len / 288;
  uint8_t *pairs = reserve_bls(count, 224);
  if (!pairs) return output_failed();
  for (uint64_t i = 0; i < count; i++) {
    compact_g2(&input, pairs + i * 224, i * 288);
    copy_padded(&input, pairs + i * 224 + 192, i * 288 + 256, 32);
  }
  uint8_t *out = reserve_output(256);
  if (!out) return output_failed();
  if (zkvm_bls12_g2_msm((const zkvm_bls12_381_g2_msm_pair *)pairs,
                        (size_t)count,
                        (zkvm_bls12_381_g2_point *)compact_result) != ZKVM_EOK)
    return output_failed();
  pad_g2(out, compact_result);
  return finish_output(256);
}

uint64_t accelerator_bls_pairing_bytes(const uint8_t *bytes, uint64_t len) {
  AcceleratorInput input;
  bool verified = false;
  if (len == 0 || len % 384 != 0 || !resolve_input(&input, bytes, len))
    return 0;
  uint64_t count = len / 384;
  uint8_t *pairs = reserve_bls(count, 288);
  if (!pairs) return 0;
  for (uint64_t i = 0; i < count; i++) {
    compact_g1(&input, pairs + i * 288, i * 384);
    compact_g2(&input, pairs + i * 288 + 96, i * 384 + 128);
  }
  if (zkvm_bls12_pairing((const zkvm_bls12_381_pairing_pair *)pairs,
                         (size_t)count, &verified) != ZKVM_EOK)
    return 0;
  return 2 | (verified ? 1 : 0);
}

bool accelerator_bls_map_fp_to_g1_bytes(const uint8_t *bytes, uint64_t len) {
  AcceleratorInput input;
  uint8_t field[48], compact_result[96];
  if (len != 64 || !resolve_input(&input, bytes, len))
    return output_failed();
  compact_fp(&input, field, 0);
  uint8_t *out = reserve_output(128);
  if (!out) return output_failed();
  if (zkvm_bls12_map_fp_to_g1((const zkvm_bls12_381_fp *)field,
                              (zkvm_bls12_381_g1_point *)compact_result) != ZKVM_EOK)
    return output_failed();
  pad_g1(out, compact_result);
  return finish_output(128);
}

bool accelerator_bls_map_fp2_to_g2_bytes(const uint8_t *bytes, uint64_t len) {
  AcceleratorInput input;
  uint8_t field[96], compact_result[192];
  if (len != 128 || !resolve_input(&input, bytes, len))
    return output_failed();
  compact_fp(&input, field, 0);
  compact_fp(&input, field + 48, 64);
  uint8_t *out = reserve_output(256);
  if (!out) return output_failed();
  if (zkvm_bls12_map_fp2_to_g2((const zkvm_bls12_381_fp2 *)field,
                               (zkvm_bls12_381_g2_point *)compact_result) != ZKVM_EOK)
    return output_failed();
  pad_g2(out, compact_result);
  return finish_output(256);
}

bool accelerator_p256_verify_bytes(const uint8_t *bytes, uint64_t len) {
  AcceleratorInput input;
  bool verified = false;
  if (len != 160 || !resolve_input(&input, bytes, len)) return false;
  return zkvm_secp256r1_verify((const zkvm_secp256r1_hash *)input.bytes,
                              (const zkvm_secp256r1_signature *)(input.bytes + 32),
                              (const zkvm_secp256r1_pubkey *)(input.bytes + 96),
                              &verified) == ZKVM_EOK && verified;
}

#ifdef EVMSAIL_DEBUG
int evmsail_debug_ecrecover_status = ZKVM_EFAIL;
uint64_t evmsail_debug_ecrecover_parity;
uint8_t evmsail_debug_ecrecover_pubkey[64];
uint8_t evmsail_debug_ecrecover_address[20];
#endif

bool precompile_ecrecover_hash_sig_address(uint8_t address[20], sail_fixed_bytes_32 hash,
                                           uint64_t yparity, sail_u256 r,
                                           sail_u256 s) {
  zkvm_secp256k1_signature signature;
  zkvm_secp256k1_pubkey public_key;
  zkvm_keccak256_hash address_hash = {{0}};
  bool ok = yparity <= 1;
  int recovery_status = ZKVM_EFAIL;
  memset(address, 0, 20);
  memset(&public_key, 0, sizeof public_key);
  sail_word_to_be_bytes(signature.data, r);
  sail_word_to_be_bytes(signature.data + 32, s);
  if (ok) {
    recovery_status = zkvm_secp256k1_ecrecover(
        (const zkvm_secp256k1_hash *)hash.bytes, &signature,
        (uint8_t)yparity, &public_key);
    ok = recovery_status == ZKVM_EOK;
  }
  if (ok) {
    ok = zkvm_keccak256(public_key.data, sizeof public_key.data,
                       &address_hash) == ZKVM_EOK;
  }
  if (ok) {
    memcpy(address, address_hash.data + 12, 20);
  }
#ifdef EVMSAIL_DEBUG
  evmsail_debug_ecrecover_status = recovery_status;
  evmsail_debug_ecrecover_parity = yparity;
  memcpy(evmsail_debug_ecrecover_pubkey, public_key.data,
         sizeof evmsail_debug_ecrecover_pubkey);
  memcpy(evmsail_debug_ecrecover_address, address,
         sizeof evmsail_debug_ecrecover_address);
#endif
  return ok;
}
