/*
 * Model-aware marshalling for nominal host-backed byte regions.
 *
 * The Sail values contain only {off, len}.  The function name (or the
 * CalldataSlice union arm) determines the backing allocation; there is no
 * runtime provenance tag.
 */
#include EVMSAIL_MODEL_H

#include "capacity.h"
#include "region_access.h"
#include "code_db.h"
#include "kernel_state.h"
#include "memory.h"
#include "output.h"
#include "precompiles.h"
#include "scratch.h"
#include "trie_node_db.h"
#include "value_convert.h"
#include "zkvm_io.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef const uint8_t *(*span_resolver)(uint64_t, uint64_t);

static const uint8_t *private_input;
static size_t private_input_size;
static bool private_input_ready;
static const uint8_t empty_region;


#define SLICE_OFF(slice) ((slice).zbytes)
#define SLICE_LEN(slice) ((slice).zlen)

static void acquire_private_input(void) {
  if (!private_input_ready) {
    read_input(&private_input, &private_input_size);
    size_t canonical_size = 0;
    if (!evmsail_capacity_prepare_input(private_input, private_input_size,
                                        &canonical_size))
      abort();
    private_input_size = canonical_size;
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
  const uint64_t total = (uint64_t)private_input_size;
  if (off > total || len > total - off) return NULL;
  if (len == 0) return &empty_region;
  return private_input ? private_input + off : NULL;
}

bool evmsail_stateless_input_contains(uint64_t pointer, uint64_t len) {
  acquire_private_input();
  if (len == 0) return true;
  uintptr_t base = (uintptr_t)private_input;
  uintptr_t address = (uintptr_t)pointer;
  return private_input && address >= base &&
         address - base <= private_input_size &&
         len <= private_input_size - (size_t)(address - base);
}

const uint8_t *evmsail_memory_ptr(uint64_t off, uint64_t len) {
  if (len == 0) return &empty_region;
  const uint8_t *bytes = evm_memory_region(off, len);
  return bytes;
}

const uint8_t *evmsail_code_ptr(uint64_t off, uint64_t len) {
  const uint8_t *bytes = NULL;
  uint64_t resolved_len = 0;
  if (len == 0) return &empty_region;
  if (!code_db_resolve_code(off, len, &bytes, &resolved_len) ||
      resolved_len != len) {
    return NULL;
  }
  return bytes;
}

const uint8_t *evmsail_scratch_ptr(uint64_t off, uint64_t len) {
  if (len == 0) return &empty_region;
  const uint8_t *bytes = scratch_region(off, len);
  return bytes;
}

const uint8_t *evmsail_log_data_ptr(uint64_t off, uint64_t len) {
  if (len == 0) return &empty_region;
  const uint8_t *bytes = log_data_region(off, len);
  return bytes;
}

const uint8_t *evmsail_output_ptr(uint64_t off, uint64_t len) {
  const uint8_t *bytes = NULL;
  uint64_t total = 0;
  if (len == 0) return &empty_region;
  if (!output_buffer_span(&bytes, &total)) {
    return NULL;
  }
  if (off > total || len > total - off) return NULL;
  return bytes + off;
}

#define DEFINE_SLICE_VALUE(name, type)                                       \
  static void name(struct type *out, uint64_t off, uint64_t len) {            \
    out->zbytes = off;                                                        \
    out->zlen = len;                                                          \
  }

DEFINE_SLICE_VALUE(stateless_input_value, zStatelessInputSliceFields)
DEFINE_SLICE_VALUE(memory_slice_value, zEvmMemorySliceFields)
DEFINE_SLICE_VALUE(scratch_slice_value, zScratchSliceFields)

#undef DEFINE_SLICE_VALUE

static void full_input_value(struct zStatelessInputSliceFields *out) {
  acquire_private_input();
  stateless_input_value(
      out,
      0,
      (uint64_t)private_input_size);
}

static void expanded_memory_value(struct zEvmMemorySliceFields *out,
                                  uint64_t len) {
  memory_slice_value(out, evm_memory_expand(len), len);
}

static void node_lookup_value(struct zStatelessInputSliceFields *out,
                              sail_fixed_bytes_32 hash) {
  uint64_t off = 0;
  uint64_t len = 0;
  nodedb_lookup_span(hash, &off, &len);
  stateless_input_value(out, off, len);
}

struct zStatelessInputSliceFields stateless_input(unit u) {
  struct zStatelessInputSliceFields out;
  (void)u;
  full_input_value(&out);
  return out;
}

struct zEvmMemorySliceFields mem_expand(uint64_t len) {
  struct zEvmMemorySliceFields out;
  expanded_memory_value(&out, len);
  return out;
}

struct zStatelessInputSliceFields nodedb_lookup(sail_fixed_bytes_32 hash) {
  struct zStatelessInputSliceFields out;
  node_lookup_value(&out, hash);
  return out;
}

static uint64_t region_byte_at(span_resolver resolve, uint64_t off,
                               uint64_t len, uint64_t index) {
  if (index >= len) return 0;
  const uint8_t *byte = resolve(off + index, 1);
  return byte ? *byte : 0;
}

static uint64_t region_count_nonzero(span_resolver resolve, uint64_t off,
                                     uint64_t len) {
  const uint8_t *bytes = resolve(off, len);
  if (!bytes) return 0;
  uint64_t count = 0;
  for (uint64_t i = 0; i < len; ++i) count += bytes[i] != 0;
  return count;
}

static bool region_strided_zero(span_resolver resolve, uint64_t off,
                                uint64_t len, uint64_t start,
                                uint64_t stride, uint64_t width,
                                uint64_t count) {
  if (count == 0 || width == 0) return true;
  if (start > len || width > len - start) return false;
  if (count > 1 &&
      (stride == 0 || count - 1 > (UINT64_MAX - start) / stride))
    return false;
  const uint64_t last = start + (count - 1) * stride;
  if (last > len || width > len - last) return false;
  const uint8_t *bytes = resolve(off, len);
  if (!bytes) return false;
  for (uint64_t i = 0; i < count; ++i) {
    const uint8_t *field = bytes + start + i * stride;
    for (uint64_t j = 0; j < width; ++j)
      if (field[j] != 0) return false;
  }
  return true;
}

static sail_u256 region_load_word(span_resolver resolve, uint64_t off,
                                  uint64_t len, uint64_t index) {
  uint8_t bytes[32] = {0};
  if (index < len) {
    uint64_t count = len - index;
    if (count > sizeof(bytes)) count = sizeof(bytes);
    const uint8_t *source = resolve(off + index, count);
    if (source) memcpy(bytes, source, (size_t)count);
  }
  return be_bytes_to_sail_word(bytes);
}

static sail_u256 region_load_n_word(span_resolver resolve, uint64_t off,
                                    uint64_t len, uint64_t index,
                                    uint64_t requested) {
  uint8_t bytes[32] = {0};
  const uint64_t width = requested < sizeof(bytes) ? requested : sizeof(bytes);
  if (index < len) {
    uint64_t count = len - index;
    if (count > width) count = width;
    const uint8_t *source = resolve(off + index, count);
    if (source) memcpy(bytes + sizeof(bytes) - width, source, (size_t)count);
  }
  return be_bytes_to_sail_word(bytes);
}

static unit region_copy_to_memory(span_resolver resolve, uint64_t off,
                                  uint64_t len, uint64_t dst, uint64_t index,
                                  uint64_t requested) {
  if (requested == 0) return UNIT;
  uint8_t *destination = evm_memory_write_region(dst, requested);
  if (!destination) return UNIT;
  uint64_t count = 0;
  if (index < len) {
    count = len - index;
    if (count > requested) count = requested;
    /*
     * Resolve after expanding the destination: EVM-memory expansion can move
     * its arena and must not invalidate a memory-backed source pointer.
     */
    const uint8_t *source = resolve(off + index, count);
    if (source)
      memmove(destination, source, (size_t)count);
    else
      count = 0;
  }
  memset(destination + count, 0, (size_t)(requested - count));
  return UNIT;
}

#define DEFINE_SLICE_BYTE(prefix, type, resolver)                             \
  uint64_t prefix##_byte_at(struct type slice, uint64_t index) {              \
    return region_byte_at(resolver, SLICE_OFF(slice), SLICE_LEN(slice),        \
                          index);                                              \
  }

#define DEFINE_SLICE_LOAD(prefix, type, resolver)                             \
  sail_u256 prefix##_load_word(struct type slice, uint64_t index) {            \
    return region_load_word(resolver, SLICE_OFF(slice), SLICE_LEN(slice),      \
                            index);                                             \
  }

#define DEFINE_SLICE_LOAD_N(prefix, type, resolver)                           \
  sail_u256 prefix##_load_n_word(struct type slice, uint64_t index,            \
                                 uint64_t len) {                               \
    return region_load_n_word(resolver, SLICE_OFF(slice), SLICE_LEN(slice),    \
                              index, len);                                      \
  }

#define DEFINE_SLICE_COPY(prefix, type, resolver)                             \
  unit prefix##_copy_to_memory(struct type slice, uint64_t dst,                \
                               uint64_t index, uint64_t len) {                 \
    return region_copy_to_memory(                                              \
        resolver, SLICE_OFF(slice), SLICE_LEN(slice), dst, index, len);        \
  }

DEFINE_SLICE_BYTE(stateless_input, zStatelessInputSliceFields,
                  evmsail_stateless_input_ptr)
DEFINE_SLICE_BYTE(memory_slice, zEvmMemorySliceFields, evmsail_memory_ptr)
DEFINE_SLICE_BYTE(code_region, zCodeRegionSliceFields, evmsail_code_ptr)
DEFINE_SLICE_BYTE(scratch_slice, zScratchSliceFields, evmsail_scratch_ptr)
DEFINE_SLICE_BYTE(log_data_slice, zLogDataSliceFields, evmsail_log_data_ptr)
DEFINE_SLICE_BYTE(output_slice, zOutputSliceFields, evmsail_output_ptr)

DEFINE_SLICE_LOAD(stateless_input, zStatelessInputSliceFields,
                  evmsail_stateless_input_ptr)
DEFINE_SLICE_LOAD(memory_slice, zEvmMemorySliceFields, evmsail_memory_ptr)
DEFINE_SLICE_LOAD(code_region, zCodeRegionSliceFields, evmsail_code_ptr)
DEFINE_SLICE_LOAD(scratch_slice, zScratchSliceFields, evmsail_scratch_ptr)
DEFINE_SLICE_LOAD(log_data_slice, zLogDataSliceFields, evmsail_log_data_ptr)
DEFINE_SLICE_LOAD(output_slice, zOutputSliceFields, evmsail_output_ptr)

DEFINE_SLICE_LOAD_N(stateless_input, zStatelessInputSliceFields,
                    evmsail_stateless_input_ptr)
DEFINE_SLICE_LOAD_N(code_region, zCodeRegionSliceFields, evmsail_code_ptr)
DEFINE_SLICE_LOAD_N(scratch_slice, zScratchSliceFields, evmsail_scratch_ptr)

DEFINE_SLICE_COPY(stateless_input, zStatelessInputSliceFields,
                  evmsail_stateless_input_ptr)
DEFINE_SLICE_COPY(memory_slice, zEvmMemorySliceFields, evmsail_memory_ptr)
DEFINE_SLICE_COPY(code_region, zCodeRegionSliceFields, evmsail_code_ptr)
DEFINE_SLICE_COPY(output_slice, zOutputSliceFields, evmsail_output_ptr)

#undef DEFINE_SLICE_BYTE
#undef DEFINE_SLICE_LOAD
#undef DEFINE_SLICE_LOAD_N
#undef DEFINE_SLICE_COPY

uint64_t stateless_input_count_nonzero(
    struct zStatelessInputSliceFields slice) {
  return region_count_nonzero(evmsail_stateless_input_ptr, SLICE_OFF(slice),
                              SLICE_LEN(slice));
}


bool stateless_input_strided_zero(
    struct zStatelessInputSliceFields slice,
    uint64_t start, uint64_t stride, uint64_t width, uint64_t count) {
  return region_strided_zero(
      evmsail_stateless_input_ptr, SLICE_OFF(slice), SLICE_LEN(slice),
      start, stride, width, count);
}

bool memory_slice_strided_zero(
    struct zEvmMemorySliceFields slice, uint64_t start, uint64_t stride,
    uint64_t width, uint64_t count) {
  return region_strided_zero(
      evmsail_memory_ptr, SLICE_OFF(slice), SLICE_LEN(slice),
      start, stride, width, count);
}

static bool regions_equal(span_resolver left_resolver, uint64_t left_off,
                          uint64_t left_len, span_resolver right_resolver,
                          uint64_t right_off, uint64_t right_len) {
  if (left_len != right_len) return false;
  const uint8_t *left = left_resolver(left_off, left_len);
  const uint8_t *right = right_resolver(right_off, right_len);
  return left && right && memcmp(left, right, (size_t)left_len) == 0;
}

bool scratch_input_slices_equal(struct zScratchSliceFields left,
                                struct zStatelessInputSliceFields right) {
  return regions_equal(evmsail_scratch_ptr, SLICE_OFF(left), SLICE_LEN(left),
                       evmsail_stateless_input_ptr, SLICE_OFF(right),
                       SLICE_LEN(right));
}

bool log_input_slices_equal(struct zLogDataSliceFields left,
                            struct zStatelessInputSliceFields right) {
  return regions_equal(evmsail_log_data_ptr, SLICE_OFF(left), SLICE_LEN(left),
                       evmsail_stateless_input_ptr, SLICE_OFF(right),
                       SLICE_LEN(right));
}

bool input_code_slices_equal(struct zStatelessInputSliceFields left,
                             struct zCodeRegionSliceFields right) {
  return regions_equal(evmsail_stateless_input_ptr, SLICE_OFF(left),
                       SLICE_LEN(left), evmsail_code_ptr, SLICE_OFF(right),
                       SLICE_LEN(right));
}

bool evmsail_logs_bloom_matches_ref(
    sail_fixed_bytes_256 computed, struct zStatelessInputSliceFields reference) {
  const uint64_t reference_len = SLICE_LEN(reference);
  const uint8_t *reference_bytes =
      evmsail_stateless_input_ptr(SLICE_OFF(reference), reference_len);
  if (reference_len != sizeof(computed.bytes) || !reference_bytes) return false;

  for (size_t word = 0; word < sizeof(computed.bytes) / sizeof(uint64_t);
       ++word) {
    uint64_t computed_word = 0;
    uint64_t reference_word = 0;
    const size_t reference_offset =
        sizeof(computed.bytes) - (word + 1) * sizeof(uint64_t);
    memcpy(&computed_word, computed.bytes + word * sizeof(uint64_t),
           sizeof(computed_word));
    memcpy(&reference_word, reference_bytes + reference_offset,
           sizeof(reference_word));
    if (computed_word != __builtin_bswap64(reference_word)) return false;
  }
  return true;
}

static void scratch_result_value(struct zScratchSliceFields *result,
                                 uint64_t end) {
  scratch_slice_value(result, 0, end);
}

void scratch_store_byte(struct zScratchSliceFields *result, uint64_t off,
                        uint64_t data) {
  const uint64_t dst = off;
  uint8_t *out = scratch_prepare(dst, 1);
  if (!out) abort();
  out[0] = (uint8_t)data;
  if (!scratch_commit(dst, 1)) abort();
  scratch_result_value(result, dst + 1);
}

static void scratch_store_region(struct zScratchSliceFields *result,
                                 uint64_t dst, span_resolver resolve,
                                 uint64_t off, uint64_t len) {
  if (len > UINT64_MAX - dst) abort();
  uint8_t *out = scratch_prepare(dst, len);
  if (len != 0 && !out) abort();
  const uint8_t *source = resolve(off, len);
  if (!source) abort();
  if (len != 0) memmove(out, source, (size_t)len);
  if (!scratch_commit(dst, len)) abort();
  scratch_result_value(result, dst + len);
}

#define DEFINE_SCRATCH_STORE(name, type, resolver)                            \
  void name(struct zScratchSliceFields *result,                               \
            uint64_t off, struct type slice) {                                \
    scratch_store_region(result, off, resolver,                               \
                         SLICE_OFF(slice), SLICE_LEN(slice));                  \
  }

DEFINE_SCRATCH_STORE(scratch_store_stateless_input,
                     zStatelessInputSliceFields, evmsail_stateless_input_ptr)
DEFINE_SCRATCH_STORE(scratch_store_scratch, zScratchSliceFields,
                     evmsail_scratch_ptr)
DEFINE_SCRATCH_STORE(scratch_store_log_data, zLogDataSliceFields,
                     evmsail_log_data_ptr)
DEFINE_SCRATCH_STORE(scratch_store_output, zOutputSliceFields,
                     evmsail_output_ptr)

#undef DEFINE_SCRATCH_STORE

void scratch_store_address(struct zScratchSliceFields *result,
                           uint64_t off, sail_fixed_bytes_20 data) {
  const uint64_t dst = off;
  uint8_t *out = scratch_prepare(dst, 20);
  if (!out) abort();
  memcpy(out, data.bytes, 20);
  if (!scratch_commit(dst, 20)) abort();
  scratch_result_value(result, dst + 20);
}

void scratch_store_b256(struct zScratchSliceFields *result,
                        uint64_t off, sail_fixed_bytes_32 data,
                        uint64_t len) {
  const uint64_t dst = off;
  if (len > 32) abort();
  uint8_t *out = scratch_prepare(dst, len);
  if (len != 0 && !out) abort();
  if (len != 0) memcpy(out, data.bytes, (size_t)len);
  if (!scratch_commit(dst, len)) abort();
  scratch_result_value(result, dst + len);
}

void scratch_store_fixed_bytes_256(struct zScratchSliceFields *result,
                                   uint64_t off,
                                   sail_fixed_bytes_256 data) {
  const uint64_t dst = off;
  uint8_t *out = scratch_prepare(dst, 256);
  if (!out) abort();
  for (size_t i = 0; i < 256; ++i) out[i] = data.bytes[255 - i];
  if (!scratch_commit(dst, 256)) abort();
  scratch_result_value(result, dst + 256);
}

void scratch_store_receipt_logs_bloom(
    struct zScratchSliceFields *result,
    uint64_t off, uint64_t start, uint64_t count) {
  const uint64_t dst = off;
  uint8_t *out = scratch_prepare(dst, 256);
  if (!out || !evmsail_receipt_logs_bloom_write(start, count, out)) abort();
  if (!scratch_commit(dst, 256)) abort();
  scratch_result_value(result, dst + 256);
}

void scratch_store_word(struct zScratchSliceFields *result,
                        uint64_t off,
                        const sail_u256 data, uint64_t len) {
  const uint64_t dst = off;
  if (len > 32) abort();
  uint8_t *out = scratch_prepare(dst, len);
  if (len != 0 && !out) abort();
  for (uint64_t i = 0; i < len; ++i) {
    const uint64_t byte_from_low = len - 1 - i;
    out[i] = (uint8_t)((data).limbs[byte_from_low / 8] >>
                       (8 * (byte_from_low % 8)));
  }
  if (!scratch_commit(dst, len)) abort();
  scratch_result_value(result, dst + len);
}

bool public_output_write(struct zScratchSliceFields output) {
  const uint64_t len = SLICE_LEN(output);
  const uint8_t *bytes = evmsail_scratch_ptr(SLICE_OFF(output), len);
  if (!bytes || len > SIZE_MAX) return false;
  write_output(bytes, (size_t)len);
  return true;
}

bool output_buffer_store_memory(struct zEvmMemorySliceFields slice) {
  return output_buffer_store_bytes(
      evmsail_memory_ptr(SLICE_OFF(slice), SLICE_LEN(slice)),
      SLICE_LEN(slice));
}

bool output_buffer_store_input(struct zStatelessInputSliceFields slice) {
  return output_buffer_store_bytes(
      evmsail_stateless_input_ptr(SLICE_OFF(slice), SLICE_LEN(slice)),
      SLICE_LEN(slice));
}

sail_fixed_bytes_32 code_db_store_indexed(struct zCodeFields code) {
  return code_db_store_indexed_bytes(
      evmsail_code_ptr(code.zbytes, code.zlen), code.zlen, code.zjumpdests);
}

static bool calldata_span(struct zCalldataSlice input,
                          const uint8_t **bytes, uint64_t *len) {
  switch (input.kind) {
    case Kind_zInputCalldata:
      *len = SLICE_LEN(input.variants.zInputCalldata);
      *bytes = evmsail_stateless_input_ptr(
          SLICE_OFF(input.variants.zInputCalldata), *len);
      return *bytes != NULL;
    case Kind_zMemoryCalldata:
      *len = SLICE_LEN(input.variants.zMemoryCalldata);
      *bytes =
          evmsail_memory_ptr(SLICE_OFF(input.variants.zMemoryCalldata), *len);
      return *bytes != NULL;
  }
  return false;
}

#define CALL_ACCELERATOR(input, call)                                        \
  do {                                                                        \
    const uint8_t *bytes = NULL;                                               \
    uint64_t len = 0;                                                          \
    if (!calldata_span((input), &bytes, &len)) return false;                   \
    return call;                                                               \
  } while (0)

bool accelerator_ripemd160(struct zCalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_ripemd160_bytes(bytes, len));
}

bool accelerator_modexp(struct zCalldataSlice input,
                        uint64_t base_len, uint64_t exponent_len,
                        uint64_t modulus_len) {
  CALL_ACCELERATOR(
      input,
      accelerator_modexp_bytes(
          bytes, len, base_len, exponent_len, modulus_len));
}

bool accelerator_bn254_add(struct zCalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_bn254_add_bytes(bytes, len));
}

bool accelerator_bn254_mul(struct zCalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_bn254_mul_bytes(bytes, len));
}

uint64_t accelerator_bn254_pairing(struct zCalldataSlice input) {
  const uint8_t *bytes = NULL;
  uint64_t len = 0;
  if (!calldata_span(input, &bytes, &len)) return 0;
  return accelerator_bn254_pairing_bytes(bytes, len);
}

bool accelerator_blake2f(struct zCalldataSlice input, uint64_t rounds,
                         uint64_t final_block) {
  CALL_ACCELERATOR(
      input,
      accelerator_blake2f_bytes(bytes, len, rounds, final_block));
}

bool accelerator_kzg_point_evaluation(struct zCalldataSlice input) {
  CALL_ACCELERATOR(
      input, accelerator_kzg_point_evaluation_bytes(bytes, len));
}

bool accelerator_bls_g1_add(struct zCalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_bls_g1_add_bytes(bytes, len));
}

bool accelerator_bls_g1_msm(struct zCalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_bls_g1_msm_bytes(bytes, len));
}

bool accelerator_bls_g2_add(struct zCalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_bls_g2_add_bytes(bytes, len));
}

bool accelerator_bls_g2_msm(struct zCalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_bls_g2_msm_bytes(bytes, len));
}

uint64_t accelerator_bls_pairing(struct zCalldataSlice input) {
  const uint8_t *bytes = NULL;
  uint64_t len = 0;
  if (!calldata_span(input, &bytes, &len)) return 0;
  return accelerator_bls_pairing_bytes(bytes, len);
}

bool accelerator_bls_map_fp_to_g1(struct zCalldataSlice input) {
  CALL_ACCELERATOR(
      input, accelerator_bls_map_fp_to_g1_bytes(bytes, len));
}

bool accelerator_bls_map_fp2_to_g2(struct zCalldataSlice input) {
  CALL_ACCELERATOR(
      input, accelerator_bls_map_fp2_to_g2_bytes(bytes, len));
}

bool accelerator_p256_verify(struct zCalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_p256_verify_bytes(bytes, len));
}

#undef CALL_ACCELERATOR
#undef SLICE_OFF
#undef SLICE_LEN
