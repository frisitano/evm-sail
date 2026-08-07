#ifndef EVMSAIL_OPTIMIZED_HOST_REGION_ACCESS_H
#define EVMSAIL_OPTIMIZED_HOST_REGION_ACCESS_H

#include "evmsail/prelude.h"
#include "evmsail/host/types.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * The generated model owns all aggregate layouts. Optimized byte-region
 * slices carry {pointer, len}; analyzed Code records additionally carry the
 * completed JUMPDEST byte pointer.
 */
bool stateless_input_offset(const uint8_t *pointer, size_t len, uint32_t *off);
#ifdef EVMSAIL_NATIVE_TEST
/* native-harness reset surface */
void input_reset(void);
#endif

uint64_t code_region_byte_at(Bytes slice, uint32_t index);

bool input_code_slices_equal(Bytes left, Bytes right);

Bytes region_scratch_store_receipt_logs_bloom(uint64_t off, uint64_t start, uint64_t count);

#define EVMSAIL_DECLARE_SLICE_LOAD(prefix) u256 prefix##_load_word(Bytes slice, uint32_t index)

#define EVMSAIL_DECLARE_SLICE_LOAD_N(prefix)                                                       \
  u256 prefix##_load_n_word(Bytes slice, uint32_t index, uint8_t len)

#define EVMSAIL_DECLARE_SLICE_COPY(prefix)                                                         \
  void prefix##_copy_to_memory(Bytes slice, uint32_t dst, uint32_t index, uint32_t len)

EVMSAIL_DECLARE_SLICE_LOAD(stateless_input);
EVMSAIL_DECLARE_SLICE_LOAD(memory_slice);
EVMSAIL_DECLARE_SLICE_LOAD(code_region);
EVMSAIL_DECLARE_SLICE_LOAD(scratch_slice);
EVMSAIL_DECLARE_SLICE_LOAD(log_data_slice);
EVMSAIL_DECLARE_SLICE_LOAD(output_slice);

EVMSAIL_DECLARE_SLICE_LOAD_N(stateless_input);
EVMSAIL_DECLARE_SLICE_LOAD_N(code_region);
EVMSAIL_DECLARE_SLICE_LOAD_N(scratch_slice);

EVMSAIL_DECLARE_SLICE_COPY(stateless_input);
EVMSAIL_DECLARE_SLICE_COPY(memory_slice);
EVMSAIL_DECLARE_SLICE_COPY(code_region);
EVMSAIL_DECLARE_SLICE_COPY(output_slice);

#undef EVMSAIL_DECLARE_SLICE_LOAD
#undef EVMSAIL_DECLARE_SLICE_LOAD_N
#undef EVMSAIL_DECLARE_SLICE_COPY

Bytes scratch_store_fixed_bytes_256(uint64_t off, bytes256 data);

#endif
