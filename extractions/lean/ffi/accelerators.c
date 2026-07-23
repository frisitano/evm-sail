#include <lean/lean.h>

#include "../../../ffi/zkvm_accelerators.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ACCELERATOR_INPUT_MAX (1u << 21)

static lean_object *none(lean_object *input) {
  lean_dec(input);
  return lean_box(0);
}

static lean_object *some(lean_object *input, const uint8_t *bytes, size_t len) {
  lean_object *array = lean_alloc_sarray(1, len, len);
  if (len) memcpy(lean_sarray_cptr(array), bytes, len);
  lean_object *result = lean_alloc_ctor(1, 1, 0);
  lean_ctor_set(result, 0, array);
  lean_dec(input);
  return result;
}

static lean_object *checked_result(lean_object *input, int status,
                                   const uint8_t *bytes, size_t len) {
  return status == ZKVM_EOK ? some(input, bytes, len) : none(input);
}

static uint8_t input_byte(const uint8_t *input, size_t input_len,
                          size_t index) {
  return index < input_len ? input[index] : 0;
}

static void copy_padded(const uint8_t *input, size_t input_len, uint8_t *out,
                        size_t off, size_t len) {
  for (size_t i = 0; i < len; i++)
    out[i] = input_byte(input, input_len, off + i);
}

static uint8_t *materialize_padded(const uint8_t *input, size_t input_len,
                                   size_t padded_len) {
  uint8_t *bytes = malloc(padded_len ? padded_len : 1);
  if (!bytes) return NULL;
  size_t copied = input_len < padded_len ? input_len : padded_len;
  if (copied) memcpy(bytes, input, copied);
  if (padded_len > copied) memset(bytes + copied, 0, padded_len - copied);
  return bytes;
}

/* EIP-2537 uses 64-byte-padded Fp values; the accelerator ABI uses compact
 * 48-byte values. The Sail model validates the zero padding before calling. */
static void compact_fp(const uint8_t *input, size_t input_len, uint8_t *out,
                       size_t off) {
  copy_padded(input, input_len, out, off + 16, 48);
}

static void pad_fp(uint8_t *out, size_t off, const uint8_t *compact) {
  memset(out + off, 0, 16);
  memcpy(out + off + 16, compact, 48);
}

static void compact_g1(const uint8_t *input, size_t input_len, uint8_t *out,
                       size_t off) {
  compact_fp(input, input_len, out, off);
  compact_fp(input, input_len, out + 48, off + 64);
}

static void pad_g1(uint8_t *out, const uint8_t *compact) {
  pad_fp(out, 0, compact);
  pad_fp(out, 64, compact + 48);
}

/* blst serializes Fp2 components c1||c0 while EIP-2537 uses c0||c1. */
static void compact_g2(const uint8_t *input, size_t input_len, uint8_t *out,
                       size_t off) {
  compact_fp(input, input_len, out + 48, off);
  compact_fp(input, input_len, out, off + 64);
  compact_fp(input, input_len, out + 144, off + 128);
  compact_fp(input, input_len, out + 96, off + 192);
}

static void pad_g2(uint8_t *out, const uint8_t *compact) {
  pad_fp(out, 0, compact + 48);
  pad_fp(out, 64, compact);
  pad_fp(out, 128, compact + 144);
  pad_fp(out, 192, compact + 96);
}

lean_object *lean_evmsail_accelerate_bytes(uint8_t selector,
                                           lean_object *input_object,
                                           uint64_t first, uint64_t second,
                                           uint64_t third) {
  const uint8_t *input = lean_sarray_cptr(input_object);
  size_t input_len = lean_sarray_size(input_object);

  switch (selector) {
    case 0: {
      zkvm_keccak256_hash output;
      return checked_result(
          input_object, zkvm_keccak256(input, input_len, &output), output.data,
          sizeof output.data);
    }
    case 1: {
      zkvm_sha256_hash output;
      return checked_result(input_object,
                            zkvm_sha256(input, input_len, &output), output.data,
                            sizeof output.data);
    }
    case 2: {
      if (input_len > ACCELERATOR_INPUT_MAX) return none(input_object);
      zkvm_ripemd160_hash output;
      return checked_result(
          input_object, zkvm_ripemd160(input, input_len, &output), output.data,
          sizeof output.data);
    }
    case 3: {
      if (first > UINT32_MAX || second > UINT32_MAX || third > UINT32_MAX ||
          first > UINT64_MAX - 96 || second > UINT64_MAX - 96 - first ||
          third > UINT64_MAX - 96 - first - second)
        return none(input_object);
      uint64_t padded_len_u64 = 96 + first + second + third;
      if (padded_len_u64 > ACCELERATOR_INPUT_MAX) return none(input_object);
      size_t padded_len = (size_t)padded_len_u64;
      uint8_t *padded = materialize_padded(input, input_len, padded_len);
      uint8_t *output = malloc(third ? (size_t)third : 1);
      if (!padded || !output) {
        free(padded);
        free(output);
        return none(input_object);
      }
      int status = zkvm_modexp(
          padded + 96, (size_t)first, padded + 96 + first, (size_t)second,
          padded + 96 + first + second, (size_t)third, output);
      lean_object *result =
          checked_result(input_object, status, output, (size_t)third);
      free(padded);
      free(output);
      return result;
    }
    case 4: {
      uint8_t encoded[128], output[64];
      copy_padded(input, input_len, encoded, 0, sizeof encoded);
      return checked_result(
          input_object,
          zkvm_bn254_g1_add((const zkvm_bn254_g1_point *)encoded,
                            (const zkvm_bn254_g1_point *)(encoded + 64),
                            (zkvm_bn254_g1_point *)output),
          output, sizeof output);
    }
    case 5: {
      uint8_t encoded[96], output[64];
      copy_padded(input, input_len, encoded, 0, sizeof encoded);
      return checked_result(
          input_object,
          zkvm_bn254_g1_mul((const zkvm_bn254_g1_point *)encoded,
                            (const zkvm_bn254_scalar *)(encoded + 64),
                            (zkvm_bn254_g1_point *)output),
          output, sizeof output);
    }
    case 6: {
      if (input_len > ACCELERATOR_INPUT_MAX || input_len % 192 != 0)
        return none(input_object);
      bool verified = false;
      int status = zkvm_bn254_pairing(
          (const zkvm_bn254_pairing_pair *)input, input_len / 192, &verified);
      uint8_t output = verified ? 1 : 0;
      return checked_result(input_object, status, &output, 1);
    }
    case 7: {
      if (input_len != 213 || first > UINT32_MAX || second > 1)
        return none(input_object);
      uint8_t state[64], message[128], counter[16];
      copy_padded(input, input_len, state, 4, sizeof state);
      copy_padded(input, input_len, message, 68, sizeof message);
      copy_padded(input, input_len, counter, 196, sizeof counter);
      int status =
          zkvm_blake2f((uint32_t)first, (zkvm_blake2f_state *)state,
                       (const zkvm_blake2f_message *)message,
                       (const zkvm_blake2f_offset *)counter, (uint8_t)second);
      return checked_result(input_object, status, state, sizeof state);
    }
    case 8: {
      if (input_len != 192) return none(input_object);
      bool verified = false;
      int status = zkvm_kzg_point_eval(
          (const zkvm_kzg_commitment *)(input + 96),
          (const zkvm_kzg_field_element *)(input + 32),
          (const zkvm_kzg_field_element *)(input + 64),
          (const zkvm_kzg_proof *)(input + 144), &verified);
      uint8_t output = status == ZKVM_EOK && verified ? 1 : 0;
      return some(input_object, &output, 1);
    }
    case 9: {
      if (input_len != 256) return none(input_object);
      uint8_t encoded[192], compact[96], output[128];
      compact_g1(input, input_len, encoded, 0);
      compact_g1(input, input_len, encoded + 96, 128);
      int status = zkvm_bls12_g1_add(
          (const zkvm_bls12_381_g1_point *)encoded,
          (const zkvm_bls12_381_g1_point *)(encoded + 96),
          (zkvm_bls12_381_g1_point *)compact);
      if (status == ZKVM_EOK) pad_g1(output, compact);
      return checked_result(input_object, status, output, sizeof output);
    }
    case 10: {
      if (input_len == 0 || input_len > ACCELERATOR_INPUT_MAX ||
          input_len % 160 != 0)
        return none(input_object);
      size_t count = input_len / 160;
      uint8_t *pairs = malloc(count * 128);
      uint8_t compact[96], output[128];
      if (!pairs) return none(input_object);
      for (size_t i = 0; i < count; i++) {
        compact_g1(input, input_len, pairs + i * 128, i * 160);
        copy_padded(input, input_len, pairs + i * 128 + 96,
                    i * 160 + 128, 32);
      }
      int status = zkvm_bls12_g1_msm(
          (const zkvm_bls12_381_g1_msm_pair *)pairs, count,
          (zkvm_bls12_381_g1_point *)compact);
      free(pairs);
      if (status == ZKVM_EOK) pad_g1(output, compact);
      return checked_result(input_object, status, output, sizeof output);
    }
    case 11: {
      if (input_len != 512) return none(input_object);
      uint8_t encoded[384], compact[192], output[256];
      compact_g2(input, input_len, encoded, 0);
      compact_g2(input, input_len, encoded + 192, 256);
      int status = zkvm_bls12_g2_add(
          (const zkvm_bls12_381_g2_point *)encoded,
          (const zkvm_bls12_381_g2_point *)(encoded + 192),
          (zkvm_bls12_381_g2_point *)compact);
      if (status == ZKVM_EOK) pad_g2(output, compact);
      return checked_result(input_object, status, output, sizeof output);
    }
    case 12: {
      if (input_len == 0 || input_len > ACCELERATOR_INPUT_MAX ||
          input_len % 288 != 0)
        return none(input_object);
      size_t count = input_len / 288;
      uint8_t *pairs = malloc(count * 224);
      uint8_t compact[192], output[256];
      if (!pairs) return none(input_object);
      for (size_t i = 0; i < count; i++) {
        compact_g2(input, input_len, pairs + i * 224, i * 288);
        copy_padded(input, input_len, pairs + i * 224 + 192,
                    i * 288 + 256, 32);
      }
      int status = zkvm_bls12_g2_msm(
          (const zkvm_bls12_381_g2_msm_pair *)pairs, count,
          (zkvm_bls12_381_g2_point *)compact);
      free(pairs);
      if (status == ZKVM_EOK) pad_g2(output, compact);
      return checked_result(input_object, status, output, sizeof output);
    }
    case 13: {
      if (input_len == 0 || input_len > ACCELERATOR_INPUT_MAX ||
          input_len % 384 != 0)
        return none(input_object);
      size_t count = input_len / 384;
      uint8_t *pairs = malloc(count * 288);
      if (!pairs) return none(input_object);
      for (size_t i = 0; i < count; i++) {
        compact_g1(input, input_len, pairs + i * 288, i * 384);
        compact_g2(input, input_len, pairs + i * 288 + 96,
                   i * 384 + 128);
      }
      bool verified = false;
      int status = zkvm_bls12_pairing(
          (const zkvm_bls12_381_pairing_pair *)pairs, count, &verified);
      free(pairs);
      uint8_t output = verified ? 1 : 0;
      return checked_result(input_object, status, &output, 1);
    }
    case 14: {
      if (input_len != 64) return none(input_object);
      uint8_t field[48], compact[96], output[128];
      compact_fp(input, input_len, field, 0);
      int status = zkvm_bls12_map_fp_to_g1(
          (const zkvm_bls12_381_fp *)field,
          (zkvm_bls12_381_g1_point *)compact);
      if (status == ZKVM_EOK) pad_g1(output, compact);
      return checked_result(input_object, status, output, sizeof output);
    }
    case 15: {
      if (input_len != 128) return none(input_object);
      uint8_t field[96], compact[192], output[256];
      compact_fp(input, input_len, field, 0);
      compact_fp(input, input_len, field + 48, 64);
      int status = zkvm_bls12_map_fp2_to_g2(
          (const zkvm_bls12_381_fp2 *)field,
          (zkvm_bls12_381_g2_point *)compact);
      if (status == ZKVM_EOK) pad_g2(output, compact);
      return checked_result(input_object, status, output, sizeof output);
    }
    case 16: {
      if (input_len != 160) return none(input_object);
      bool verified = false;
      int status = zkvm_secp256r1_verify(
          (const zkvm_secp256r1_hash *)input,
          (const zkvm_secp256r1_signature *)(input + 32),
          (const zkvm_secp256r1_pubkey *)(input + 96), &verified);
      uint8_t output = status == ZKVM_EOK && verified ? 1 : 0;
      return some(input_object, &output, 1);
    }
    case 17: {
      if (input_len != 160) return none(input_object);
      bool verified = false;
      int status = zkvm_secp256k1_verify(
          (const zkvm_secp256k1_hash *)input,
          (const zkvm_secp256k1_signature *)(input + 32),
          (const zkvm_secp256k1_pubkey *)(input + 96), &verified);
      uint8_t output = status == ZKVM_EOK && verified ? 1 : 0;
      return some(input_object, &output, 1);
    }
    case 18: {
      if (input_len != 96 || first > 1) return none(input_object);
      zkvm_secp256k1_pubkey public_key;
      int status = zkvm_secp256k1_ecrecover(
          (const zkvm_secp256k1_hash *)input,
          (const zkvm_secp256k1_signature *)(input + 32), (uint8_t)first,
          &public_key);
      if (status != ZKVM_EOK) return none(input_object);
      zkvm_keccak256_hash digest;
      status = zkvm_keccak256(public_key.data, sizeof public_key.data, &digest);
      return checked_result(input_object, status, digest.data + 12, 20);
    }
    default:
      return none(input_object);
  }
}
