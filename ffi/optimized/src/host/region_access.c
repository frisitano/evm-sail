/*
 * Model-aware marshalling for nominal host-backed byte regions.
 *
 * The Sail values contain only {off, len}.  The function name (or the
 * CalldataSlice union arm) determines the backing allocation; there is no
 * runtime provenance tag.
 */
#include "evmsail/prelude.h"

#include "evmsail/host/region_access.h"
#include "host/code/store.h"
#include "host/state/runtime.h"
#include "evmsail/host/memory.h"
#include "evmsail/host/output.h"
#include "evmsail/host/accelerators.h"
#include "evmsail/host/scratch.h"
#include "evmsail/host/nodes.h"
#include "primitives/value.h"
#include "workspace.h"
#include "zkvm_io.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

enum RegionKind {
  REGION_STATELESS_INPUT,
  REGION_MEMORY,
  REGION_CODE,
  REGION_SCRATCH,
  REGION_LOG_DATA,
  REGION_OUTPUT,
};

static const uint8_t *private_input;
static size_t private_input_size;
static bool private_input_ready;
static const uint8_t empty_region;


#define SLICE_OFF(slice) ((slice).off)
#define SLICE_LEN(slice) ((slice).len)

static void acquire_private_input(void) {
  if (!private_input_ready) {
    read_input(&private_input, &private_input_size);
    private_input_ready = true;
  }
}

void input_reset(void) {
  private_input = NULL;
  private_input_size = 0;
  private_input_ready = false;
}

const uint8_t *stateless_input_ptr(uint64_t off, uint64_t len) {
  acquire_private_input();
  const uint64_t total = (uint64_t)private_input_size;
  if (off > total || len > total - off) return NULL;
  if (len == 0) return &empty_region;
  return private_input ? private_input + off : NULL;
}

bool stateless_input_contains(uint64_t pointer, uint64_t len) {
  acquire_private_input();
  if (len == 0) return true;
  uintptr_t base = (uintptr_t)private_input;
  uintptr_t address = (uintptr_t)pointer;
  return private_input && address >= base &&
         address - base <= private_input_size &&
         len <= private_input_size - (size_t)(address - base);
}

bool stateless_input_offset(const uint8_t *pointer, uint64_t len,
                                    uint64_t *off) {
  acquire_private_input();
  if (!off || !private_input || !pointer) return false;
  const uintptr_t base = (uintptr_t)private_input;
  const uintptr_t address = (uintptr_t)pointer;
  if (address < base || address - base > private_input_size ||
      len > private_input_size - (size_t)(address - base))
    return false;
  *off = (uint64_t)(address - base);
  return true;
}

const uint8_t *memory_ptr(uint64_t off, uint64_t len) {
  if (len == 0) return &empty_region;
  const uint8_t *bytes = evm_memory_region(off, len);
  return bytes;
}

const uint8_t *code_ptr(uint64_t off, uint64_t len) {
  const uint8_t *bytes = NULL;
  uint64_t resolved_len = 0;
  if (len == 0) return &empty_region;
  if (!code_db_resolve_code(off, len, &bytes, &resolved_len) ||
      resolved_len != len)
    GUEST_ABORT();
  return bytes;
}

const uint8_t *scratch_ptr(uint64_t off, uint64_t len) {
  if (len == 0) return &empty_region;
  const uint8_t *bytes = scratch_region(off, len);
  return bytes;
}

const uint8_t *log_data_ptr(uint64_t off, uint64_t len) {
  if (len == 0) return &empty_region;
  const uint8_t *bytes = log_data_region(off, len);
  return bytes;
}

const uint8_t *output_ptr(uint64_t off, uint64_t len) {
  const uint8_t *bytes = NULL;
  uint64_t total = 0;
  if (len == 0) return &empty_region;
  if (!output_buffer_span(&bytes, &total)) {
    return NULL;
  }
  if (off > total || len > total - off) return NULL;
  return bytes + off;
}

/* Region provenance is a closed semantic set. Carry a small tag through the
 * shared operations rather than an indirect C callback; constant call sites
 * are then specialized by the C compiler. */
static const uint8_t *resolve_region(enum RegionKind kind, uint64_t off,
                                     uint64_t len) {
  switch (kind) {
    case REGION_STATELESS_INPUT:
      return stateless_input_ptr(off, len);
    case REGION_MEMORY:
      return memory_ptr(off, len);
    case REGION_CODE:
      return code_ptr(off, len);
    case REGION_SCRATCH:
      return scratch_ptr(off, len);
    case REGION_LOG_DATA:
      return log_data_ptr(off, len);
    case REGION_OUTPUT:
      return output_ptr(off, len);
  }
  return NULL;
}

#define DEFINE_SLICE_VALUE(name, type)                                       \
  static void name(struct type *out, uint64_t off, uint64_t len) {            \
    out->off = off;                                                          \
    out->len = len;                                                          \
  }

DEFINE_SLICE_VALUE(stateless_input_value, StatelessInputSliceFields)
DEFINE_SLICE_VALUE(memory_slice_value, EvmMemorySliceFields)
DEFINE_SLICE_VALUE(scratch_slice_value, ScratchSliceFields)

#undef DEFINE_SLICE_VALUE

static void full_input_value(struct StatelessInputSliceFields *out) {
  acquire_private_input();
  stateless_input_value(
      out,
      0,
      (uint64_t)private_input_size);
}

static void expanded_memory_value(struct EvmMemorySliceFields *out,
                                  uint64_t len) {
  memory_slice_value(out, evm_memory_expand(len), len);
}

static void node_lookup_value(struct StatelessInputSliceFields *out,
                              Hash32 hash) {
  uint64_t off = 0;
  uint64_t len = 0;
  nodedb_lookup_span(hash, &off, &len);
  stateless_input_value(out, off, len);
}

struct StatelessInputSliceFields stateless_input(unit u) {
  struct StatelessInputSliceFields out;
  (void)u;
  full_input_value(&out);
  return out;
}

struct EvmMemorySliceFields mem_expand(uint32_t len) {
  struct EvmMemorySliceFields out;
  expanded_memory_value(&out, len);
  return out;
}

struct StatelessInputSliceFields nodedb_lookup(Hash32 hash) {
  struct StatelessInputSliceFields out;
  node_lookup_value(&out, hash);
  return out;
}

struct OutputSliceFields region_output_buffer_slice(uint64_t len) {
  struct OutputSliceFields out;
  const uint8_t *bytes = NULL;
  uint64_t total = 0;
  if (!output_buffer_span(&bytes, &total) || len > total) GUEST_ABORT();
  out.off = 0;
  out.len = len;
  return out;
}

static uint64_t region_byte_at(enum RegionKind kind, uint64_t off,
                               uint64_t len, uint64_t index) {
  if (index >= len) return 0;
  const uint8_t *bytes = resolve_region(kind, off, len);
  return bytes ? bytes[index] : 0;
}

static uint64_t region_count_nonzero(enum RegionKind kind, uint64_t off,
                                     uint64_t len) {
  const uint8_t *bytes = resolve_region(kind, off, len);
  if (!bytes) return 0;
  uint64_t count = 0;
  for (uint64_t i = 0; i < len; ++i) count += bytes[i] != 0;
  return count;
}

static bool region_strided_zero(enum RegionKind kind, uint64_t off,
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
  const uint8_t *bytes = resolve_region(kind, off, len);
  if (!bytes) return false;
  for (uint64_t i = 0; i < count; ++i) {
    const uint8_t *field = bytes + start + i * stride;
    for (uint64_t j = 0; j < width; ++j)
      if (field[j] != 0) return false;
  }
  return true;
}

static U256 region_load_word(enum RegionKind kind, uint64_t off,
                                  uint64_t len, uint64_t index) {
  uint8_t bytes[32] = {0};
  if (index < len) {
    uint64_t count = len - index;
    if (count > sizeof(bytes)) count = sizeof(bytes);
    const uint8_t *source = resolve_region(kind, off, len);
    if (source) memcpy(bytes, source + index, (size_t)count);
  }
  return be_bytes_to_sail_word(bytes);
}

static U256 region_load_n_word(enum RegionKind kind, uint64_t off,
                                    uint64_t len, uint64_t index,
                                    uint64_t requested) {
  uint8_t bytes[32] = {0};
  const uint64_t width = requested < sizeof(bytes) ? requested : sizeof(bytes);
  if (index < len) {
    uint64_t count = len - index;
    if (count > width) count = width;
    const uint8_t *source = resolve_region(kind, off, len);
    if (source)
      memcpy(bytes + sizeof(bytes) - width, source + index, (size_t)count);
  }
  return be_bytes_to_sail_word(bytes);
}

static unit region_copy_to_memory(enum RegionKind kind, uint64_t off,
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
    const uint8_t *source = resolve_region(kind, off, len);
    if (source)
      memmove(destination, source + index, (size_t)count);
    else
      count = 0;
  }
  memset(destination + count, 0, (size_t)(requested - count));
  return UNIT;
}

#define DEFINE_SLICE_BYTE(prefix, type, kind)                                 \
  uint64_t prefix##_byte_at(struct type slice, uint32_t index) {              \
    return region_byte_at(kind, SLICE_OFF(slice), SLICE_LEN(slice),            \
                          index);                                              \
  }

#define DEFINE_SLICE_LOAD(prefix, type, kind)                                 \
  U256 prefix##_load_word(struct type slice, uint32_t index) {            \
    return region_load_word(kind, SLICE_OFF(slice), SLICE_LEN(slice),          \
                            index);                                             \
  }

#define DEFINE_SLICE_LOAD_N(prefix, type, kind)                               \
  U256 prefix##_load_n_word(struct type slice, uint32_t index,            \
                                 uint8_t len) {                                \
    return region_load_n_word(kind, SLICE_OFF(slice), SLICE_LEN(slice),        \
                              index, len);                                      \
  }

#define DEFINE_SLICE_COPY(prefix, type, kind)                                 \
  unit prefix##_copy_to_memory(struct type slice, uint32_t dst,                \
                               uint32_t index, uint32_t len) {                 \
    return region_copy_to_memory(                                              \
        kind, SLICE_OFF(slice), SLICE_LEN(slice), dst, index, len);            \
  }

DEFINE_SLICE_BYTE(stateless_input, StatelessInputSliceFields,
                  REGION_STATELESS_INPUT)
DEFINE_SLICE_BYTE(memory_slice, EvmMemorySliceFields, REGION_MEMORY)
DEFINE_SLICE_BYTE(code_region, CodeRegionSliceFields, REGION_CODE)
DEFINE_SLICE_BYTE(scratch_slice, ScratchSliceFields, REGION_SCRATCH)
DEFINE_SLICE_BYTE(log_data_slice, LogDataSliceFields, REGION_LOG_DATA)
DEFINE_SLICE_BYTE(output_slice, OutputSliceFields, REGION_OUTPUT)

DEFINE_SLICE_LOAD(stateless_input, StatelessInputSliceFields,
                  REGION_STATELESS_INPUT)
DEFINE_SLICE_LOAD(memory_slice, EvmMemorySliceFields, REGION_MEMORY)
DEFINE_SLICE_LOAD(code_region, CodeRegionSliceFields, REGION_CODE)
DEFINE_SLICE_LOAD(scratch_slice, ScratchSliceFields, REGION_SCRATCH)
DEFINE_SLICE_LOAD(log_data_slice, LogDataSliceFields, REGION_LOG_DATA)
DEFINE_SLICE_LOAD(output_slice, OutputSliceFields, REGION_OUTPUT)

DEFINE_SLICE_LOAD_N(stateless_input, StatelessInputSliceFields,
                    REGION_STATELESS_INPUT)
DEFINE_SLICE_LOAD_N(code_region, CodeRegionSliceFields, REGION_CODE)
DEFINE_SLICE_LOAD_N(scratch_slice, ScratchSliceFields, REGION_SCRATCH)

DEFINE_SLICE_COPY(stateless_input, StatelessInputSliceFields,
                  REGION_STATELESS_INPUT)
DEFINE_SLICE_COPY(memory_slice, EvmMemorySliceFields, REGION_MEMORY)
DEFINE_SLICE_COPY(code_region, CodeRegionSliceFields, REGION_CODE)
DEFINE_SLICE_COPY(output_slice, OutputSliceFields, REGION_OUTPUT)

#undef DEFINE_SLICE_BYTE
#undef DEFINE_SLICE_LOAD
#undef DEFINE_SLICE_LOAD_N
#undef DEFINE_SLICE_COPY

uint32_t stateless_input_count_nonzero(
    struct StatelessInputSliceFields slice) {
  return (uint32_t)region_count_nonzero(
      REGION_STATELESS_INPUT, SLICE_OFF(slice), SLICE_LEN(slice));
}

bool stateless_input_strided_zero(
    struct StatelessInputSliceFields slice,
    uint32_t start, uint32_t stride, uint32_t width, uint32_t count) {
  return region_strided_zero(
      REGION_STATELESS_INPUT, SLICE_OFF(slice), SLICE_LEN(slice),
      start, stride, width, count);
}

bool memory_slice_strided_zero(
    struct EvmMemorySliceFields slice, uint32_t start, uint32_t stride,
    uint32_t width, uint32_t count) {
  return region_strided_zero(
      REGION_MEMORY, SLICE_OFF(slice), SLICE_LEN(slice),
      start, stride, width, count);
}

static bool regions_equal(enum RegionKind left_kind, uint64_t left_off,
                          uint64_t left_len, enum RegionKind right_kind,
                          uint64_t right_off, uint64_t right_len) {
  if (left_len != right_len) return false;
  const uint8_t *left = resolve_region(left_kind, left_off, left_len);
  const uint8_t *right = resolve_region(right_kind, right_off, right_len);
  return left && right && memcmp(left, right, (size_t)left_len) == 0;
}

bool scratch_input_slices_equal(struct ScratchSliceFields left,
                                struct StatelessInputSliceFields right) {
  return regions_equal(REGION_SCRATCH, SLICE_OFF(left), SLICE_LEN(left),
                       REGION_STATELESS_INPUT, SLICE_OFF(right),
                       SLICE_LEN(right));
}

bool log_input_slices_equal(struct LogDataSliceFields left,
                            struct StatelessInputSliceFields right) {
  return regions_equal(REGION_LOG_DATA, SLICE_OFF(left), SLICE_LEN(left),
                       REGION_STATELESS_INPUT, SLICE_OFF(right),
                       SLICE_LEN(right));
}

bool input_code_slices_equal(struct StatelessInputSliceFields left,
                             struct CodeRegionSliceFields right) {
  return regions_equal(REGION_STATELESS_INPUT, SLICE_OFF(left),
                       SLICE_LEN(left), REGION_CODE, SLICE_OFF(right),
                       SLICE_LEN(right));
}

bool region_logs_bloom_matches_ref(
    LogsBloom computed, struct StatelessInputSliceFields reference) {
  const uint64_t reference_len = SLICE_LEN(reference);
  const uint8_t *reference_bytes =
      stateless_input_ptr(SLICE_OFF(reference), reference_len);
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

static struct ScratchRegionResult scratch_result_value(bool accepted,
                                                        uint64_t end) {
  struct ScratchRegionResult result = {0};
  if (!accepted) {
    result.kind = Kind_ScratchRegionFailed;
    result.variants.ScratchRegionFailed = UNIT;
    return result;
  }
  result.kind = Kind_ScratchRegionReady;
  scratch_slice_value(
      &result.variants.ScratchRegionReady,
      0,
      end);
  return result;
}

struct ScratchRegionResult scratch_store_byte(uint32_t off, uint64_t data) {
  const uint64_t dst = off;
  uint8_t *out = scratch_prepare(dst, 1);
  if (!out) return scratch_result_value(false, 0);
  out[0] = (uint8_t)data;
  const bool accepted = scratch_commit(dst, 1);
  return scratch_result_value(accepted, accepted ? dst + 1 : 0);
}

static struct ScratchRegionResult scratch_store_region(
    uint64_t dst, enum RegionKind kind, uint64_t off, uint64_t len) {
  if (len > UINT64_MAX - dst) {
    return scratch_result_value(false, 0);
  }
  uint8_t *out = scratch_prepare(dst, len);
  if (len != 0 && !out) {
    return scratch_result_value(false, 0);
  }
  const uint8_t *source = resolve_region(kind, off, len);
  if (!source) {
    return scratch_result_value(false, 0);
  }
  if (len != 0) memmove(out, source, (size_t)len);
  const bool accepted = scratch_commit(dst, len);
  return scratch_result_value(accepted, accepted ? dst + len : 0);
}

#define DEFINE_SCRATCH_STORE(name, type, kind)                                \
  struct ScratchRegionResult name(uint32_t off, struct type slice) {          \
    return scratch_store_region(off, kind, SLICE_OFF(slice),                  \
                                SLICE_LEN(slice));                            \
  }

DEFINE_SCRATCH_STORE(scratch_store_stateless_input,
                     StatelessInputSliceFields, REGION_STATELESS_INPUT)
DEFINE_SCRATCH_STORE(scratch_store_scratch, ScratchSliceFields,
                     REGION_SCRATCH)
DEFINE_SCRATCH_STORE(scratch_store_log_data, LogDataSliceFields,
                     REGION_LOG_DATA)
DEFINE_SCRATCH_STORE(scratch_store_output, OutputSliceFields,
                     REGION_OUTPUT)

#undef DEFINE_SCRATCH_STORE

struct ScratchRegionResult scratch_store_address(uint32_t off, Address data) {
  const uint64_t dst = off;
  uint8_t *out = scratch_prepare(dst, 20);
  if (!out) return scratch_result_value(false, 0);
  memcpy(out, address_bytes_const(&data), 20);
  const bool accepted = scratch_commit(dst, 20);
  return scratch_result_value(accepted, accepted ? dst + 20 : 0);
}

struct ScratchRegionResult scratch_store_b256(uint32_t off, Hash32 data,
                                               uint8_t len) {
  const uint64_t dst = off;
  if (len > 32) {
    return scratch_result_value(false, 0);
  }
  uint8_t *out = scratch_prepare(dst, len);
  if (len != 0 && !out) {
    return scratch_result_value(false, 0);
  }
  if (len != 0)
    memcpy(out, hash_bytes_const(&data), (size_t)len);
  const bool accepted = scratch_commit(dst, len);
  return scratch_result_value(accepted, accepted ? dst + len : 0);
}

void scratch_store_fixed_bytes_256(struct ScratchRegionResult *result,
                                   uint64_t off,
                                   LogsBloom data) {
  const uint64_t dst = off;
  uint8_t *out = scratch_prepare(dst, 256);
  if (!out) {
    *result = scratch_result_value(false, 0);
    return;
  }
  for (size_t i = 0; i < 256; ++i) out[i] = data.bytes[255 - i];
  const bool accepted = scratch_commit(dst, 256);
  *result = scratch_result_value(accepted, accepted ? dst + 256 : 0);
}

void region_scratch_store_receipt_logs_bloom(
    struct ScratchRegionResult *result,
    uint64_t off, uint64_t start, uint64_t count) {
  const uint64_t dst = off;
  uint8_t *out = scratch_prepare(dst, 256);
  if (!out || !receipt_runtime_bloom_write(start, count, out)) {
    *result = scratch_result_value(false, 0);
    return;
  }
  const bool accepted = scratch_commit(dst, 256);
  *result = scratch_result_value(accepted, accepted ? dst + 256 : 0);
}

struct ScratchRegionResult scratch_store_word(uint32_t off,
                                               const U256 data,
                                               uint8_t len) {
  const uint64_t dst = off;
  if (len > 32) {
    return scratch_result_value(false, 0);
  }
  uint8_t *out = scratch_prepare(dst, len);
  if (len != 0 && !out) {
    return scratch_result_value(false, 0);
  }
  for (uint64_t i = 0; i < len; ++i) {
    const uint64_t byte_from_low = len - 1 - i;
    out[i] = (uint8_t)((data).limbs[byte_from_low / 8] >>
                       (8 * (byte_from_low % 8)));
  }
  const bool accepted = scratch_commit(dst, len);
  return scratch_result_value(accepted, accepted ? dst + len : 0);
}

bool public_output_write(struct ScratchSliceFields output) {
  const uint64_t len = SLICE_LEN(output);
  const uint8_t *bytes = scratch_ptr(SLICE_OFF(output), len);
  if (!bytes || len > SIZE_MAX) return false;
  write_output(bytes, (size_t)len);
  return true;
}

bool output_buffer_store_memory(struct EvmMemorySliceFields slice) {
  return output_buffer_store_bytes(
      memory_ptr(SLICE_OFF(slice), SLICE_LEN(slice)),
      SLICE_LEN(slice));
}

bool output_buffer_store_input(struct StatelessInputSliceFields slice) {
  return output_buffer_store_bytes(
      stateless_input_ptr(SLICE_OFF(slice), SLICE_LEN(slice)),
      SLICE_LEN(slice));
}

Hash32 code_db_store_indexed(
    struct CodeRegionSliceFields code, uint64_t jumpdest_ref) {
  return code_db_store_row(SLICE_OFF(code), jumpdest_ref);
}

static bool calldata_span(struct CalldataSlice input,
                          const uint8_t **bytes, uint64_t *len) {
  switch (input.kind) {
    case Kind_InputCalldata:
      *len = SLICE_LEN(input.variants.InputCalldata);
      *bytes = stateless_input_ptr(
          SLICE_OFF(input.variants.InputCalldata), *len);
      return *bytes != NULL;
    case Kind_MemoryCalldata:
      *len = SLICE_LEN(input.variants.MemoryCalldata);
      *bytes =
          memory_ptr(SLICE_OFF(input.variants.MemoryCalldata), *len);
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

bool accelerator_ripemd160(struct CalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_ripemd160_bytes(bytes, len));
}

bool accelerator_modexp(struct CalldataSlice input,
                        uint32_t base_len, uint32_t exponent_len,
                        uint32_t modulus_len) {
  CALL_ACCELERATOR(
      input,
      accelerator_modexp_bytes(
          bytes, len, base_len, exponent_len, modulus_len));
}

bool accelerator_bn254_add(struct CalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_bn254_add_bytes(bytes, len));
}

bool accelerator_bn254_mul(struct CalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_bn254_mul_bytes(bytes, len));
}

uint8_t accelerator_bn254_pairing(struct CalldataSlice input) {
  const uint8_t *bytes = NULL;
  uint64_t len = 0;
  if (!calldata_span(input, &bytes, &len)) return 0;
  return accelerator_bn254_pairing_bytes(bytes, len);
}

bool accelerator_blake2f(struct CalldataSlice input, uint32_t rounds,
                         uint8_t final_block) {
  CALL_ACCELERATOR(
      input,
      accelerator_blake2f_bytes(bytes, len, rounds, final_block));
}

bool accelerator_kzg_point_evaluation(struct CalldataSlice input) {
  CALL_ACCELERATOR(
      input, accelerator_kzg_point_evaluation_bytes(bytes, len));
}

bool accelerator_bls_g1_add(struct CalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_bls_g1_add_bytes(bytes, len));
}

bool accelerator_bls_g1_msm(struct CalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_bls_g1_msm_bytes(bytes, len));
}

bool accelerator_bls_g2_add(struct CalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_bls_g2_add_bytes(bytes, len));
}

bool accelerator_bls_g2_msm(struct CalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_bls_g2_msm_bytes(bytes, len));
}

uint8_t accelerator_bls_pairing(struct CalldataSlice input) {
  const uint8_t *bytes = NULL;
  uint64_t len = 0;
  if (!calldata_span(input, &bytes, &len)) return 0;
  return accelerator_bls_pairing_bytes(bytes, len);
}

bool accelerator_bls_map_fp_to_g1(struct CalldataSlice input) {
  CALL_ACCELERATOR(
      input, accelerator_bls_map_fp_to_g1_bytes(bytes, len));
}

bool accelerator_bls_map_fp2_to_g2(struct CalldataSlice input) {
  CALL_ACCELERATOR(
      input, accelerator_bls_map_fp2_to_g2_bytes(bytes, len));
}

bool accelerator_p256_verify(struct CalldataSlice input) {
  CALL_ACCELERATOR(input, accelerator_p256_verify_bytes(bytes, len));
}

#undef CALL_ACCELERATOR
#undef SLICE_OFF
#undef SLICE_LEN
