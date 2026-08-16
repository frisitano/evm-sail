/*
 * Model-aware marshalling for nominal host-backed byte regions.
 *
 * Sail retains semantic coordinates. Optimized C lowers every byte-region
 * field to a direct stable pointer.
 */
#include "evmsail/prelude.h"

#ifdef EVMSAIL_NATIVE_TEST
#include "evmsail/exceptions.h"
#endif
#include "evmsail/host/region_access.h"
#include "evmsail/spec/exceptions.h"
#include "host/state/logs.h"
#include "evmsail/host/memory.h"
#include "evmsail/host/output.h"
#include "evmsail/host/scratch.h"
#include "evmsail/host/nodes.h"
#include "primitives/value.h"
#include "workspace.h"
#include "zkvm_io.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static const uint8_t *private_input;
static size_t private_input_size;
static bool private_input_ready;

#define SLICE_PTR(slice) ((slice).bytes)
#define SLICE_LEN(slice) ((slice).len)

static void acquire_private_input(void)
{
  if (!private_input_ready) {
    read_input(&private_input, &private_input_size);
    private_input_ready = true;
  }
}

#ifdef EVMSAIL_NATIVE_TEST
/* native-harness reset surface */
void input_reset(void)
{
  fatal_error_reset();
  private_input = NULL;
  private_input_size = 0;
  private_input_ready = false;
}
#endif

bool stateless_input_offset(const uint8_t *pointer, size_t len, uint32_t *off)
{
  acquire_private_input();
  if (!off || !private_input || !pointer || private_input_size > UINT32_MAX) {
    return false;
  }
  const uintptr_t base = (uintptr_t)private_input;
  const uintptr_t address = (uintptr_t)pointer;
  if (address < base || address - base > private_input_size ||
      len > private_input_size - (size_t)(address - base)) {
    return false;
  }
  *off = (uint32_t)(address - base);
  return true;
}

static void full_input_value(Bytes *out)
{
  acquire_private_input();
  if (private_input_size > UINT32_MAX) {
    fatal_error(InvalidConfig);
  }
  out->bytes = private_input_size == 0 ? NULL : private_input;
  out->len = (uint32_t)private_input_size;
}

Bytes stateless_input(void)
{
  Bytes out;
  full_input_value(&out);
  return out;
}

Bytes mem_expand(uint32_t pointer, uint32_t established, uint32_t required)
{
  Bytes out;
  const uint64_t base = evm_memory_expand(pointer, established, required);
  out.bytes = required == 0 ? NULL : evm_memory_base() + base;
  out.len = required;
  return out;
}

Bytes nodedb_lookup(bytes32 hash)
{
  Bytes out;
  uint32_t off = 0;
  uint32_t len = 0;
  /* Spans were validated against the input at insert time. */
  nodedb_lookup_span(hash, &off, &len);
  acquire_private_input();
  out.bytes = len == 0 ? NULL : private_input + off;
  out.len = len;
  return out;
}

/* Sail slice_byte wrappers call only under off < s.len
 * (sail/host/region_access.sail); a nonempty slice carries a resolved
 * pointer. */
static uint64_t region_byte_at(const uint8_t *bytes, uint64_t len, uint64_t index)
{
  (void)len;
  return bytes[index];
}

static uint64_t region_count_nonzero(const uint8_t *bytes, uint64_t len)
{
  uint64_t count = 0;
  for (uint64_t i = 0; i < len; ++i) {
    count += bytes[i] != 0;
  }
  return count;
}

static bool region_strided_zero(const uint8_t *bytes, uint64_t len, uint64_t start, uint64_t stride,
                                uint64_t width, uint64_t count)
{
  if (count == 0 || width == 0) {
    return true;
  }
  if (start > len || width > len - start) {
    return false;
  }
  /* stride == 0 with count > 1 stays false for spec-backend parity. */
  if (count > 1 && stride == 0) {
    return false;
  }
  const uint64_t last = start + ((count - 1) * stride);
  if (last > len || width > len - last) {
    return false;
  }
  for (uint64_t i = 0; i < count; ++i) {
    const uint8_t *field = bytes + start + (i * stride);
    for (uint64_t j = 0; j < width; ++j) {
      if (field[j] != 0) {
        return false;
      }
    }
  }
  return true;
}

/* Sail slice_load wrappers call only under index < s.len
 * (sail/host/region_access.sail). */
static u256 region_load_word(const uint8_t *source, uint64_t len, uint64_t index)
{
  uint8_t bytes[32] = {0};
  uint64_t count = len - index;
  if (count > sizeof(bytes)) {
    count = sizeof(bytes);
  }
  memcpy(bytes, source + index, (size_t)count);
  return be_bytes_to_sail_word(bytes);
}

/* Sail slice_load_n wrappers call only under index < s.len with
 * 0 <= 'n <= 32 (sail/host/region_access.sail). */
static inline __attribute__((__always_inline__)) u256 region_load_n_word(const uint8_t *source,
                                                                         uint64_t len,
                                                                         uint64_t index,
                                                                         uint64_t requested)
{
  uint64_t count = len - index;
  if (count > requested) {
    count = requested;
  }

  /* The ordinary case has the complete requested field. Build Sail's
   * little-endian limb representation directly from the big-endian source,
   * avoiding a zeroed 32-byte staging buffer and a second four-lane load.
   * This is particularly important for PUSH immediates and RLP quantities. */
  if (count == requested) {
    u256 result = {{0}};
    const uint8_t *const begin = source + index;
    const uint64_t full_limbs = requested >> 3;
    const uint64_t leading = requested & 7;
    const uint8_t *cursor = begin + requested;

    if (full_limbs >= 1) {
      cursor -= 8;
      result.limbs[0] = bswap64(load_u64(cursor));
    }
    if (full_limbs >= 2) {
      cursor -= 8;
      result.limbs[1] = bswap64(load_u64(cursor));
    }
    if (full_limbs >= 3) {
      cursor -= 8;
      result.limbs[2] = bswap64(load_u64(cursor));
    }
    if (full_limbs >= 4) {
      cursor -= 8;
      result.limbs[3] = bswap64(load_u64(cursor));
    }
    if (leading != 0) {
      uint64_t partial = 0;
      memcpy(&partial, begin, (size_t)leading);
      result.limbs[full_limbs] = bswap64(partial) >> ((8 - leading) * 8);
    }
    return result;
  }

  /* A PUSH may run past the end of code. Preserve the semantic right-hand
   * zero padding for that uncommon path. */
  uint8_t bytes[32] = {0};
  memcpy(bytes + sizeof(bytes) - requested, source + index, (size_t)count);
  return be_bytes_to_sail_word(bytes);
}

static void region_copy_to_memory(const uint8_t *source, uint64_t len, uint64_t dst, uint64_t index,
                                  uint64_t requested)
{
  if (requested == 0) {
    return;
  }
  uint8_t *destination = evm_memory_write_region(dst, requested);
  uint64_t count = 0;
  if (index < len) {
    /* index < len means the slice is nonempty, so source is resolved. */
    count = len - index;
    if (count > requested) {
      count = requested;
    }
    memmove(destination, source + index, (size_t)count);
  }
  memset(destination + count, 0, (size_t)(requested - count));
}

#define DEFINE_SLICE_BYTE(prefix)                                                                  \
  uint64_t prefix##_byte_at(Bytes slice, uint32_t index)                                           \
  {                                                                                                \
    return region_byte_at(SLICE_PTR(slice), SLICE_LEN(slice), index);                              \
  }

#define DEFINE_SLICE_LOAD(prefix)                                                                  \
  u256 prefix##_load_word(Bytes slice, uint32_t index)                                             \
  {                                                                                                \
    return region_load_word(SLICE_PTR(slice), SLICE_LEN(slice), index);                            \
  }

#define DEFINE_SLICE_LOAD_N(prefix)                                                                \
  u256 prefix##_load_n_word(Bytes slice, uint32_t index, uint8_t len)                              \
  {                                                                                                \
    return region_load_n_word(SLICE_PTR(slice), SLICE_LEN(slice), index, len);                     \
  }

#define DEFINE_SLICE_COPY(prefix)                                                                  \
  void prefix##_copy_to_memory(Bytes slice, uint32_t dst, uint32_t index, uint32_t len)            \
  {                                                                                                \
    region_copy_to_memory(SLICE_PTR(slice), SLICE_LEN(slice), dst, index, len);                    \
  }

DEFINE_SLICE_BYTE(stateless_input)
DEFINE_SLICE_BYTE(memory_slice)
DEFINE_SLICE_BYTE(scratch_slice)
DEFINE_SLICE_BYTE(log_data_slice)
DEFINE_SLICE_BYTE(output_slice)

DEFINE_SLICE_LOAD(stateless_input)
DEFINE_SLICE_LOAD(memory_slice)
DEFINE_SLICE_LOAD(scratch_slice)
DEFINE_SLICE_LOAD(log_data_slice)
DEFINE_SLICE_LOAD(output_slice)

DEFINE_SLICE_LOAD_N(stateless_input)
DEFINE_SLICE_LOAD_N(scratch_slice)

DEFINE_SLICE_COPY(stateless_input)
DEFINE_SLICE_COPY(memory_slice)
DEFINE_SLICE_COPY(output_slice)

#undef DEFINE_SLICE_BYTE
#undef DEFINE_SLICE_LOAD
#undef DEFINE_SLICE_LOAD_N
#undef DEFINE_SLICE_COPY

uint64_t code_region_byte_at(Bytes slice, uint32_t index)
{
  return region_byte_at(slice.bytes, slice.len, index);
}

u256 code_region_load_word(Bytes slice, uint32_t index)
{
  return region_load_word(slice.bytes, slice.len, index);
}

u256 code_region_load_n_word(Bytes slice, uint32_t index, uint8_t len)
{
  return region_load_n_word(slice.bytes, slice.len, index, len);
}

void code_region_copy_to_memory(Bytes slice, uint32_t dst, uint32_t index, uint32_t len)
{
  region_copy_to_memory(slice.bytes, slice.len, dst, index, len);
}

uint32_t stateless_input_count_nonzero(Bytes slice)
{
  return (uint32_t)region_count_nonzero(SLICE_PTR(slice), SLICE_LEN(slice));
}

bool stateless_input_strided_zero(Bytes slice, uint32_t start, uint32_t stride, uint32_t width,
                                  uint32_t count)
{
  return region_strided_zero(SLICE_PTR(slice), SLICE_LEN(slice), start, stride, width, count);
}

bool memory_slice_strided_zero(Bytes slice, uint32_t start, uint32_t stride, uint32_t width,
                               uint32_t count)
{
  return region_strided_zero(SLICE_PTR(slice), SLICE_LEN(slice), start, stride, width, count);
}

static bool regions_equal(const uint8_t *left, uint64_t left_len, const uint8_t *right,
                          uint64_t right_len)
{
  if (left_len != right_len) {
    return false;
  }
  if (left_len == 0) {
    return true;
  }
  return memcmp(left, right, (size_t)left_len) == 0;
}

bool scratch_input_slices_equal(Bytes left, Bytes right)
{
  return regions_equal(SLICE_PTR(left), SLICE_LEN(left), SLICE_PTR(right), SLICE_LEN(right));
}

bool log_input_slices_equal(Bytes left, Bytes right)
{
  return regions_equal(SLICE_PTR(left), SLICE_LEN(left), SLICE_PTR(right), SLICE_LEN(right));
}

bool input_code_slices_equal(Bytes left, Bytes right)
{
  return regions_equal(SLICE_PTR(left), SLICE_LEN(left), right.bytes, right.len);
}

/* end <= GUEST_SCRATCH_BYTES (16 MiB), proved by scratch_prepare. */
static Bytes scratch_result_value(uint64_t end)
{
  return (Bytes){
      .bytes = end == 0 ? NULL : scratch_base(),
      .len = (uint32_t)end,
  };
}

Bytes scratch_store_byte(uint32_t off, uint64_t data)
{
  const uint64_t dst = off;
  uint8_t *out = scratch_prepare(dst, 1);
  if (!out) {
    fatal_error(InvalidConfig);
  }
  out[0] = (uint8_t)data;
  scratch_commit(dst, 1);
  return scratch_result_value(dst + 1);
}

static Bytes scratch_store_region(uint64_t dst, const uint8_t *source, uint64_t len)
{
  uint8_t *out = scratch_prepare(dst, len);
  if (len != 0 && !out) {
    fatal_error(InvalidConfig);
  }
  if (len != 0) {
    memmove(out, source, (size_t)len);
  }
  scratch_commit(dst, len);
  return scratch_result_value(dst + len);
}

#define DEFINE_SCRATCH_STORE(name)                                                                 \
  Bytes name(uint32_t off, Bytes slice)                                                            \
  {                                                                                                \
    return scratch_store_region(off, SLICE_PTR(slice), SLICE_LEN(slice));                          \
  }

DEFINE_SCRATCH_STORE(scratch_store_stateless_input)
DEFINE_SCRATCH_STORE(scratch_store_scratch)
DEFINE_SCRATCH_STORE(scratch_store_log_data)
DEFINE_SCRATCH_STORE(scratch_store_output)

#undef DEFINE_SCRATCH_STORE

Bytes scratch_store_address(uint32_t off, bytes20 data)
{
  const uint64_t dst = off;
  uint8_t *out = scratch_prepare(dst, 20);
  if (!out) {
    fatal_error(InvalidConfig);
  }
  memcpy(out, bytes20_data(&data), 20);
  scratch_commit(dst, 20);
  return scratch_result_value(dst + 20);
}

/* Sail host_scratch_store_b256: 0 <= 'len <= 32 (sail/host/scratch.sail). */
Bytes scratch_store_b256(uint32_t off, bytes32 data, uint8_t len)
{
  const uint64_t dst = off;
  uint8_t *out = scratch_prepare(dst, len);
  if (len != 0 && !out) {
    fatal_error(InvalidConfig);
  }
  if (len != 0) {
    memcpy(out, bytes32_data(&data), (size_t)len);
  }
  scratch_commit(dst, len);
  return scratch_result_value(dst + len);
}

Bytes scratch_store_fixed_bytes_256(uint64_t off, bytes256 data)
{
  const uint64_t dst = off;
  uint8_t *out = scratch_prepare(dst, 256);
  if (!out) {
    fatal_error(InvalidConfig);
  }
  for (size_t i = 0; i < 256; ++i) {
    out[i] = data.bytes[255 - i];
  }
  scratch_commit(dst, 256);
  return scratch_result_value(dst + 256);
}

Bytes region_scratch_store_receipt_logs_bloom(uint64_t off, uint64_t start, uint64_t count)
{
  const uint64_t dst = off;
  uint8_t *out = scratch_prepare(dst, 256);
  if (!out || !receipt_runtime_bloom_write(start, count, out)) {
    fatal_error(InvalidConfig);
  }
  scratch_commit(dst, 256);
  return scratch_result_value(dst + 256);
}

/* Sail host_scratch_store_word: 0 <= 'len <= 32 (sail/host/scratch.sail). */
Bytes scratch_store_word(uint32_t off, const u256 data, uint8_t len)
{
  const uint64_t dst = off;
  uint8_t *out = scratch_prepare(dst, len);
  if (len != 0 && !out) {
    fatal_error(InvalidConfig);
  }
  for (uint64_t i = 0; i < len; ++i) {
    const uint64_t byte_from_low = len - 1 - i;
    out[i] = (uint8_t)(data.limbs[byte_from_low / 8] >> (8 * (byte_from_low % 8)));
  }
  scratch_commit(dst, len);
  return scratch_result_value(dst + len);
}

bool public_output_write(Bytes output)
{
  write_output(SLICE_PTR(output), (size_t)SLICE_LEN(output));
  return true;
}

bool output_buffer_store_memory(Bytes slice)
{
  return output_buffer_store_bytes(SLICE_PTR(slice), SLICE_LEN(slice));
}

bool output_buffer_store_input(Bytes slice)
{
  return output_buffer_store_bytes(SLICE_PTR(slice), SLICE_LEN(slice));
}

#undef SLICE_PTR
#undef SLICE_LEN
