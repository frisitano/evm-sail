/* ByteSlice is a generated Sail aggregate. Keep its exact layout at this one
 * model-aware boundary and pass the complete value from Sail; scalar C
 * mechanisms continue to consume the host source kind and span internally. */
#include EVMSAIL_MODEL_H

#include "byte_slice_glue.h"
#include "code_db.h"
#include "kernel_state.h"
#include "value_convert.h"
#include "memory.h"
#include "output.h"
#include "precompiles.h"
#include "scratch.h"
#include "trie_node_db.h"
#include "zkvm_io.h"

#include <stddef.h>
#include <string.h>

static const uint8_t *private_input;
static size_t private_input_size;
static bool private_input_ready;
static const uint8_t empty_input;

static uint64_t byte_slice_off(const struct zByteSlice *slice) {
  return evmsail_byte_quantity_value(slice->zoff);
}

static uint64_t byte_slice_len(const struct zByteSlice *slice) {
  return evmsail_byte_quantity_value(slice->zlen);
}

static void acquire_private_input(void) {
  if (!private_input_ready) {
    read_input(&private_input, &private_input_size);
    private_input_ready = true;
  }
}

void evmsail_input_reset(void) {
  private_input = NULL;
  private_input_size = 0;
  private_input_ready = false;
}

const uint8_t *evmsail_stateless_input_ptr(uint64_t off, uint64_t len) {
  acquire_private_input();
  uint64_t total = (uint64_t)private_input_size;
  if (off > total || len > total - off) return NULL;
  if (len == 0) return &empty_input;
  return private_input ? private_input + off : NULL;
}

static void byte_slice_value(struct zByteSlice *out, enum zByteSource source,
                             uint64_t off, uint64_t len) {
  out->zsource = source;
  evmsail_byte_quantity_set(&out->zoff, off);
  evmsail_byte_quantity_set(&out->zlen, len);
}

static void stateless_input_value(struct zByteSlice *out) {
  acquire_private_input();
  byte_slice_value(out, zStatelessInputSource, 0,
                   (uint64_t)private_input_size);
}

static void mem_expand_value(struct zByteSlice *out, uint64_t len) {
  byte_slice_value(out, zEvmMemorySource, evm_memory_expand(len), len);
}

static void nodedb_lookup_value(struct zByteSlice *out, sail_hash hash) {
  uint64_t off = 0;
  uint64_t len = 0;
  nodedb_lookup_span(hash, &off, &len);
  byte_slice_value(out, zStatelessInputSource, off, len);
}

#ifdef EVMSAIL_STANDARD_ABI
void stateless_input(struct zByteSlice *out, unit u) {
  (void)u;
  stateless_input_value(out);
}
#else
struct zByteSlice stateless_input(unit u) {
  struct zByteSlice out;
  (void)u;
  stateless_input_value(&out);
  return out;
}
#endif

#ifdef EVMSAIL_STANDARD_ABI
void mem_expand(struct zByteSlice *out,
                EVMSAIL_BYTE_QUANTITY_PARAM(len)) {
  mem_expand_value(out, evmsail_byte_quantity_value(len));
}
#else
struct zByteSlice mem_expand(EVMSAIL_BYTE_QUANTITY_PARAM(len)) {
  struct zByteSlice out;
  mem_expand_value(&out, evmsail_byte_quantity_value(len));
  return out;
}
#endif

#ifdef EVMSAIL_STANDARD_ABI
void nodedb_lookup(struct zByteSlice *out, sail_hash hash) {
  nodedb_lookup_value(out, hash);
}
#else
struct zByteSlice nodedb_lookup(sail_hash hash) {
  struct zByteSlice out;
  nodedb_lookup_value(&out, hash);
  return out;
}
#endif

uint64_t evmsail_source_kind(int generated_source) {
  switch ((enum zByteSource)generated_source) {
    case zStatelessInputSource:
      return EVMSAIL_SOURCE_STATELESS_INPUT;
    case zEvmMemorySource:
      return EVMSAIL_SOURCE_EVM_MEMORY;
    case zCodeSource:
      return EVMSAIL_SOURCE_CODE;
    case zLogDataSource:
      return EVMSAIL_SOURCE_LOG_DATA;
    case zOutputSource:
      return EVMSAIL_SOURCE_OUTPUT;
    case zScratchSource:
      return EVMSAIL_SOURCE_SCRATCH;
  }
  return 0;
}

int evmsail_resolve_byte_source(uint64_t kind, uint64_t off, uint64_t len,
                                const uint8_t **bytes,
                                uint64_t *resolved_len) {
  static const uint8_t empty_source;
  const uint8_t *source = NULL;

  if (len == 0) {
    source = &empty_source;
  } else {
    switch (kind) {
      case EVMSAIL_SOURCE_STATELESS_INPUT:
        source = evmsail_stateless_input_ptr(off, len);
        break;
      case EVMSAIL_SOURCE_EVM_MEMORY:
        source = evm_memory_region(off, len);
        break;
      case EVMSAIL_SOURCE_CODE:
        return code_db_resolve_code(off, len, bytes, resolved_len);
      case EVMSAIL_SOURCE_LOG_DATA:
        source = log_data_region(off, len);
        break;
      case EVMSAIL_SOURCE_OUTPUT: {
        const uint8_t *output = NULL;
        uint64_t output_len = 0;
        output_buffer_span(&output, &output_len);
        if (off > output_len || len > output_len - off) return 0;
        source = output + off;
        break;
      }
      case EVMSAIL_SOURCE_SCRATCH:
        source = scratch_region(off, len);
        break;
      default:
        return 0;
    }
  }

  if (!source) return 0;
  if (bytes) *bytes = source;
  if (resolved_len) *resolved_len = len;
  return 1;
}

static void scratch_result_value(struct zByteRegionResult *result,
                                 bool accepted, uint64_t end) {
  if (!accepted) {
    result->kind = Kind_zByteRegionFailed;
    result->variants.zByteRegionFailed = UNIT;
    return;
  }
  result->kind = Kind_zByteRegionReady;
#ifdef EVMSAIL_STANDARD_ABI
  /* The generated caller initializes ByteRegionResult in its inactive
     ByteRegionFailed arm.  Construct the newly selected aggregate arm before
     assigning its GMP-backed byte quantities.  The optimized ABI stores the
     same fields inline as uint64_t and requires no construction. */
  CREATE(sail_int)(&result->variants.zByteRegionReady.zoff);
  CREATE(sail_int)(&result->variants.zByteRegionReady.zlen);
#endif
  byte_slice_value(&result->variants.zByteRegionReady, zScratchSource, 0, end);
}

void scratch_store_bytes(struct zByteRegionResult *result,
                         EVMSAIL_BYTE_QUANTITY_PARAM(off),
                         evmsail_byte_list bytes,
                         EVMSAIL_BYTE_QUANTITY_PARAM(len)) {
  uint64_t off_value = evmsail_byte_quantity_value(off);
  uint64_t len_value = evmsail_byte_quantity_value(len);
  uint8_t *out = scratch_prepare(off_value, len_value);
  if (len_value != 0 && !out) {
    scratch_result_value(result, false, 0);
    return;
  }

  evmsail_byte_list byte = bytes;
  for (uint64_t i = 0; i < len_value; i++) {
    if (!byte) {
      scratch_result_value(result, false, 0);
      return;
    }
    out[i] = evmsail_byte_value(byte->hd);
    byte = byte->tl;
  }
  if (byte) {
    scratch_result_value(result, false, 0);
    return;
  }
  bool accepted = scratch_commit(off_value, len_value);
  scratch_result_value(result, accepted,
                       accepted ? off_value + len_value : 0);
}

void scratch_store_slice(struct zByteRegionResult *result,
                         EVMSAIL_BYTE_QUANTITY_PARAM(off),
                         struct zByteSlice slice) {
  uint64_t off_value = evmsail_byte_quantity_value(off);
  bool accepted = scratch_append_source(
      off_value, evmsail_source_kind(slice.zsource),
      byte_slice_off(&slice), byte_slice_len(&slice));
  scratch_result_value(result, accepted,
                       accepted ? off_value + byte_slice_len(&slice) : 0);
}

void scratch_store_address(struct zByteRegionResult *result,
                           EVMSAIL_BYTE_QUANTITY_PARAM(off),
                           sail_address data) {
  uint64_t off_value = evmsail_byte_quantity_value(off);
  uint8_t *out = scratch_prepare(off_value, 20);
  if (!out) {
    scratch_result_value(result, false, 0);
    return;
  }
  memcpy(out, data.bytes, 20);
  bool accepted = scratch_commit(off_value, 20);
  scratch_result_value(result, accepted, accepted ? off_value + 20 : 0);
}

void scratch_store_b256(struct zByteRegionResult *result,
                        EVMSAIL_BYTE_QUANTITY_PARAM(off), sail_b256 data,
                        uint64_t len) {
  uint64_t off_value = evmsail_byte_quantity_value(off);
  if (len > 32) {
    scratch_result_value(result, false, 0);
    return;
  }
  uint8_t *out = scratch_prepare(off_value, len);
  if (len != 0 && !out) {
    scratch_result_value(result, false, 0);
    return;
  }
  if (len != 0) memcpy(out, data.bytes, (size_t)len);
  bool accepted = scratch_commit(off_value, len);
  scratch_result_value(result, accepted, accepted ? off_value + len : 0);
}

bool public_output_write(struct zByteSlice output) {
  const uint8_t *bytes = NULL;
  uint64_t len = 0;
  if (!evmsail_resolve_byte_source(evmsail_source_kind(output.zsource),
                                   byte_slice_off(&output),
                                   byte_slice_len(&output), &bytes, &len) ||
      len > SIZE_MAX)
    return false;
  write_output(bytes, (size_t)len);
  return true;
}

uint64_t slice_byte_at(struct zByteSlice slice,
                       EVMSAIL_BYTE_QUANTITY_PARAM(index)) {
  return slice_byte_at_source(
      evmsail_source_kind(slice.zsource), byte_slice_off(&slice),
      byte_slice_len(&slice), evmsail_byte_quantity_value(index));
}

bool host_byte_slices_equal(struct zByteSlice left,
                            struct zByteSlice right) {
  uint64_t left_len = byte_slice_len(&left);
  uint64_t right_len = byte_slice_len(&right);
  const uint8_t *left_bytes = NULL;
  const uint8_t *right_bytes = NULL;
  uint64_t resolved_left_len = 0;
  uint64_t resolved_right_len = 0;

  if (left_len != right_len) return false;
  if (!evmsail_resolve_byte_source(
          evmsail_source_kind(left.zsource), byte_slice_off(&left), left_len,
          &left_bytes, &resolved_left_len) ||
      !evmsail_resolve_byte_source(
          evmsail_source_kind(right.zsource), byte_slice_off(&right), right_len,
          &right_bytes, &resolved_right_len) ||
      resolved_left_len != left_len || resolved_right_len != right_len)
    return false;
  return memcmp(left_bytes, right_bytes, (size_t)left_len) == 0;
}

#ifdef EVMSAIL_STANDARD_ABI
void slice_count_nonzero(sail_int *out, struct zByteSlice slice) {
  evmsail_byte_quantity_set(
      out, slice_count_nonzero_source(evmsail_source_kind(slice.zsource),
                                      byte_slice_off(&slice),
                                      byte_slice_len(&slice)));
}
#else
uint64_t slice_count_nonzero(struct zByteSlice slice) {
  return slice_count_nonzero_source(evmsail_source_kind(slice.zsource),
                                    byte_slice_off(&slice),
                                    byte_slice_len(&slice));
}
#endif

bool slice_strided_zero(struct zByteSlice slice,
                        EVMSAIL_BYTE_QUANTITY_PARAM(start),
                        EVMSAIL_BYTE_QUANTITY_PARAM(stride),
                        EVMSAIL_BYTE_QUANTITY_PARAM(width),
                        EVMSAIL_BYTE_QUANTITY_PARAM(count)) {
  return slice_strided_zero_source(
      evmsail_source_kind(slice.zsource), byte_slice_off(&slice),
      byte_slice_len(&slice), evmsail_byte_quantity_value(start),
      evmsail_byte_quantity_value(stride), evmsail_byte_quantity_value(width),
      evmsail_byte_quantity_value(count));
}

EVMSAIL_WORD_RETURN slice_load_word(EVMSAIL_WORD_RESULT(result)
                                    struct zByteSlice slice,
                                    EVMSAIL_BYTE_QUANTITY_PARAM(index)) {
  EVMSAIL_RETURN_WORD(
      result, slice_load_word_source(evmsail_source_kind(slice.zsource),
                                     byte_slice_off(&slice),
                                     byte_slice_len(&slice),
                                     evmsail_byte_quantity_value(index)));
}

EVMSAIL_WORD_RETURN slice_load_n_word(EVMSAIL_WORD_RESULT(result)
                                      struct zByteSlice slice,
                                      EVMSAIL_BYTE_QUANTITY_PARAM(index),
                                      EVMSAIL_BYTE_QUANTITY_PARAM(len)) {
  EVMSAIL_RETURN_WORD(
      result, slice_load_n_word_source(evmsail_source_kind(slice.zsource),
                                       byte_slice_off(&slice),
                                       byte_slice_len(&slice),
                                       evmsail_byte_quantity_value(index),
                                       evmsail_byte_quantity_value(len)));
}

unit slice_copy_to_memory(struct zByteSlice slice,
                          EVMSAIL_BYTE_QUANTITY_PARAM(dst),
                          EVMSAIL_BYTE_QUANTITY_PARAM(index),
                          EVMSAIL_BYTE_QUANTITY_PARAM(len)) {
  return slice_copy_to_memory_source(
      evmsail_source_kind(slice.zsource), byte_slice_off(&slice),
      byte_slice_len(&slice), evmsail_byte_quantity_value(dst),
      evmsail_byte_quantity_value(index), evmsail_byte_quantity_value(len));
}

bool output_buffer_store(struct zByteSlice slice) {
  return output_buffer_store_source(evmsail_source_kind(slice.zsource),
                                    byte_slice_off(&slice),
                                    byte_slice_len(&slice));
}

EVMSAIL_HASH_RETURN code_db_store_indexed(EVMSAIL_HASH_RESULT(result)
                                          struct zByteSlice code,
                                          uint64_t jumpdest_ref) {
  return code_db_store_indexed_source(evmsail_source_kind(code.zsource),
                                      byte_slice_off(&code),
                                      byte_slice_len(&code), jumpdest_ref);
}

bool accelerator_ripemd160(struct zByteSlice input) {
  return accelerator_ripemd160_source(evmsail_source_kind(input.zsource),
                                      byte_slice_off(&input),
                                      byte_slice_len(&input));
}

bool accelerator_modexp(struct zByteSlice input,
                        EVMSAIL_BYTE_QUANTITY_PARAM(base_len),
                        EVMSAIL_BYTE_QUANTITY_PARAM(exponent_len),
                        EVMSAIL_BYTE_QUANTITY_PARAM(modulus_len)) {
  return accelerator_modexp_source(
      evmsail_source_kind(input.zsource), byte_slice_off(&input),
      byte_slice_len(&input), evmsail_byte_quantity_value(base_len),
      evmsail_byte_quantity_value(exponent_len),
      evmsail_byte_quantity_value(modulus_len));
}

bool accelerator_bn254_add(struct zByteSlice input) {
  return accelerator_bn254_add_source(evmsail_source_kind(input.zsource),
                                      byte_slice_off(&input),
                                      byte_slice_len(&input));
}

bool accelerator_bn254_mul(struct zByteSlice input) {
  return accelerator_bn254_mul_source(evmsail_source_kind(input.zsource),
                                      byte_slice_off(&input),
                                      byte_slice_len(&input));
}

uint64_t accelerator_bn254_pairing(struct zByteSlice input) {
  return accelerator_bn254_pairing_source(evmsail_source_kind(input.zsource),
                                          byte_slice_off(&input),
                                          byte_slice_len(&input));
}

bool accelerator_blake2f(struct zByteSlice input, uint64_t rounds,
                         uint64_t final_block) {
  return accelerator_blake2f_source(
      evmsail_source_kind(input.zsource), byte_slice_off(&input),
      byte_slice_len(&input), rounds, final_block);
}

bool accelerator_kzg_point_evaluation(struct zByteSlice input) {
  return accelerator_kzg_point_evaluation_source(
      evmsail_source_kind(input.zsource), byte_slice_off(&input),
      byte_slice_len(&input));
}

bool accelerator_bls_g1_add(struct zByteSlice input) {
  return accelerator_bls_g1_add_source(evmsail_source_kind(input.zsource),
                                       byte_slice_off(&input),
                                       byte_slice_len(&input));
}

bool accelerator_bls_g1_msm(struct zByteSlice input) {
  return accelerator_bls_g1_msm_source(evmsail_source_kind(input.zsource),
                                       byte_slice_off(&input),
                                       byte_slice_len(&input));
}

bool accelerator_bls_g2_add(struct zByteSlice input) {
  return accelerator_bls_g2_add_source(evmsail_source_kind(input.zsource),
                                       byte_slice_off(&input),
                                       byte_slice_len(&input));
}

bool accelerator_bls_g2_msm(struct zByteSlice input) {
  return accelerator_bls_g2_msm_source(evmsail_source_kind(input.zsource),
                                       byte_slice_off(&input),
                                       byte_slice_len(&input));
}

uint64_t accelerator_bls_pairing(struct zByteSlice input) {
  return accelerator_bls_pairing_source(evmsail_source_kind(input.zsource),
                                        byte_slice_off(&input),
                                        byte_slice_len(&input));
}

bool accelerator_bls_map_fp_to_g1(struct zByteSlice input) {
  return accelerator_bls_map_fp_to_g1_source(
      evmsail_source_kind(input.zsource), byte_slice_off(&input),
      byte_slice_len(&input));
}

bool accelerator_bls_map_fp2_to_g2(struct zByteSlice input) {
  return accelerator_bls_map_fp2_to_g2_source(
      evmsail_source_kind(input.zsource), byte_slice_off(&input),
      byte_slice_len(&input));
}

bool accelerator_p256_verify(struct zByteSlice input) {
  return accelerator_p256_verify_source(evmsail_source_kind(input.zsource),
                                        byte_slice_off(&input),
                                        byte_slice_len(&input));
}
