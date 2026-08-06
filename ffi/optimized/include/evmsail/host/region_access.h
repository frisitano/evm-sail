#ifndef EVMSAIL_OPTIMIZED_HOST_REGION_ACCESS_H
#define EVMSAIL_OPTIMIZED_HOST_REGION_ACCESS_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * The generated model owns all aggregate layouts. Optimized byte-region
 * slices carry {pointer, len}; analyzed Code records additionally carry the
 * completed JUMPDEST byte pointer.
 */
struct StatelessInputSliceFields;
struct ScratchSliceFields;
struct EvmMemorySliceFields;
struct CodeRegionSliceFields;
struct LogDataSliceFields;
struct OutputSliceFields;

bool stateless_input_offset(const uint8_t *pointer, size_t len, uint32_t *off);
#ifdef EVMSAIL_NATIVE_TEST
/* native-harness reset surface */
void input_reset(void);
#endif

uint64_t code_region_byte_at(struct CodeRegionSliceFields slice, uint32_t index);

bool input_code_slices_equal(struct StatelessInputSliceFields left,
                             struct CodeRegionSliceFields right);

struct ScratchSliceFields region_scratch_store_receipt_logs_bloom(uint64_t off, uint64_t start,
                                                                  uint64_t count);

#define EVMSAIL_DECLARE_SLICE_LOAD(prefix, type)                                                   \
  U256 prefix##_load_word(struct type slice, uint32_t index)

#define EVMSAIL_DECLARE_SLICE_LOAD_N(prefix, type)                                                 \
  U256 prefix##_load_n_word(struct type slice, uint32_t index, uint8_t len)

#define EVMSAIL_DECLARE_SLICE_COPY(prefix, type)                                                   \
  void prefix##_copy_to_memory(struct type slice, uint32_t dst, uint32_t index, uint32_t len)

EVMSAIL_DECLARE_SLICE_LOAD(stateless_input, StatelessInputSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD(memory_slice, EvmMemorySliceFields);
EVMSAIL_DECLARE_SLICE_LOAD(code_region, CodeRegionSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD(scratch_slice, ScratchSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD(log_data_slice, LogDataSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD(output_slice, OutputSliceFields);

EVMSAIL_DECLARE_SLICE_LOAD_N(stateless_input, StatelessInputSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD_N(code_region, CodeRegionSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD_N(scratch_slice, ScratchSliceFields);

EVMSAIL_DECLARE_SLICE_COPY(stateless_input, StatelessInputSliceFields);
EVMSAIL_DECLARE_SLICE_COPY(memory_slice, EvmMemorySliceFields);
EVMSAIL_DECLARE_SLICE_COPY(code_region, CodeRegionSliceFields);
EVMSAIL_DECLARE_SLICE_COPY(output_slice, OutputSliceFields);

#undef EVMSAIL_DECLARE_SLICE_LOAD
#undef EVMSAIL_DECLARE_SLICE_LOAD_N
#undef EVMSAIL_DECLARE_SLICE_COPY

struct ScratchSliceFields scratch_store_fixed_bytes_256(uint64_t off, LogsBloom data);

#endif
