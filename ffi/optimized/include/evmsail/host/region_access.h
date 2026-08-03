#ifndef EVMSAIL_OPTIMIZED_HOST_REGION_ACCESS_H
#define EVMSAIL_OPTIMIZED_HOST_REGION_ACCESS_H

#include "evmsail/prelude.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * The generated model owns all aggregate layouts.  Each nominal slice carries
 * only {off, len}; its C function selects the backing region statically.
 */
struct StatelessInputSliceFields;
struct ScratchSliceFields;
struct EvmMemorySliceFields;
struct CodeRegionSliceFields;
struct LogDataSliceFields;
struct OutputSliceFields;
struct ScratchRegionResult;
struct CalldataSlice;

bool stateless_input_contains(uint64_t pointer, uint64_t len);
bool stateless_input_offset(const uint8_t *pointer, uint64_t len,
                                    uint64_t *off);
const uint8_t *stateless_input_ptr(uint64_t off, uint64_t len);
const uint8_t *memory_ptr(uint64_t off, uint64_t len);
const uint8_t *code_ptr(uint64_t off, uint64_t len);
const uint8_t *scratch_ptr(uint64_t off, uint64_t len);
const uint8_t *log_data_ptr(uint64_t off, uint64_t len);
const uint8_t *output_ptr(uint64_t off, uint64_t len);
void input_reset(void);

struct StatelessInputSliceFields stateless_input(unit u);
struct EvmMemorySliceFields mem_expand(uint32_t len);
struct StatelessInputSliceFields nodedb_lookup(Hash32 hash);

struct OutputSliceFields region_output_buffer_slice(uint64_t len);

uint64_t stateless_input_byte_at(
    struct StatelessInputSliceFields slice, uint32_t index);
uint64_t memory_slice_byte_at(struct EvmMemorySliceFields slice,
                              uint32_t index);
uint64_t code_region_byte_at(struct CodeRegionSliceFields slice,
                             uint32_t index);
uint64_t scratch_slice_byte_at(struct ScratchSliceFields slice,
                               uint32_t index);
uint64_t log_data_slice_byte_at(struct LogDataSliceFields slice,
                                uint32_t index);
uint64_t output_slice_byte_at(struct OutputSliceFields slice,
                              uint32_t index);

bool scratch_input_slices_equal(struct ScratchSliceFields left,
                                struct StatelessInputSliceFields right);
bool log_input_slices_equal(struct LogDataSliceFields left,
                            struct StatelessInputSliceFields right);
bool input_code_slices_equal(struct StatelessInputSliceFields left,
                             struct CodeRegionSliceFields right);
bool region_logs_bloom_matches_ref(
    LogsBloom computed, struct StatelessInputSliceFields reference);

void region_scratch_store_receipt_logs_bloom(
    struct ScratchRegionResult *result, uint64_t off, uint64_t start,
    uint64_t count);

uint32_t stateless_input_count_nonzero(
    struct StatelessInputSliceFields slice);

bool stateless_input_strided_zero(
    struct StatelessInputSliceFields slice, uint32_t start, uint32_t stride,
    uint32_t width, uint32_t count);
bool memory_slice_strided_zero(
    struct EvmMemorySliceFields slice, uint32_t start, uint32_t stride,
    uint32_t width, uint32_t count);

#define EVMSAIL_DECLARE_SLICE_LOAD(prefix, type)                              \
  U256 prefix##_load_word(                                     \
       struct type slice,                           \
      uint32_t index)

#define EVMSAIL_DECLARE_SLICE_LOAD_N(prefix, type)                            \
  U256 prefix##_load_n_word(                                   \
       struct type slice,                           \
      uint32_t index, uint8_t len)

#define EVMSAIL_DECLARE_SLICE_COPY(prefix, type)                              \
  unit prefix##_copy_to_memory(struct type slice,                             \
                               uint32_t dst, uint32_t index, uint32_t len)

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

struct ScratchRegionResult scratch_store_byte(uint32_t off, uint64_t data);
struct ScratchRegionResult scratch_store_stateless_input(
    uint32_t off, struct StatelessInputSliceFields slice);
struct ScratchRegionResult scratch_store_scratch(
    uint32_t off, struct ScratchSliceFields slice);
struct ScratchRegionResult scratch_store_log_data(
    uint32_t off, struct LogDataSliceFields slice);
struct ScratchRegionResult scratch_store_output(
    uint32_t off, struct OutputSliceFields slice);
struct ScratchRegionResult scratch_store_address(uint32_t off, Address data);
struct ScratchRegionResult scratch_store_b256(uint32_t off, Hash32 data,
                                               uint8_t len);
void scratch_store_fixed_bytes_256(struct ScratchRegionResult *result,
                                   uint64_t off,
                                   LogsBloom data);
struct ScratchRegionResult scratch_store_word(uint32_t off,
                                               const U256 data,
                                               uint8_t len);

bool public_output_write(struct ScratchSliceFields output);
bool output_buffer_store_memory(struct EvmMemorySliceFields slice);
bool output_buffer_store_input(struct StatelessInputSliceFields slice);

bool accelerator_ripemd160(struct CalldataSlice input);
bool accelerator_modexp(struct CalldataSlice input, uint32_t base_len,
                        uint32_t exponent_len, uint32_t modulus_len);
bool accelerator_bn254_add(struct CalldataSlice input);
bool accelerator_bn254_mul(struct CalldataSlice input);
uint8_t accelerator_bn254_pairing(struct CalldataSlice input);
bool accelerator_blake2f(struct CalldataSlice input, uint32_t rounds,
                         uint8_t final_block);
bool accelerator_kzg_point_evaluation(struct CalldataSlice input);
bool accelerator_bls_g1_add(struct CalldataSlice input);
bool accelerator_bls_g1_msm(struct CalldataSlice input);
bool accelerator_bls_g2_add(struct CalldataSlice input);
bool accelerator_bls_g2_msm(struct CalldataSlice input);
uint8_t accelerator_bls_pairing(struct CalldataSlice input);
bool accelerator_bls_map_fp_to_g1(struct CalldataSlice input);
bool accelerator_bls_map_fp2_to_g2(struct CalldataSlice input);
bool accelerator_p256_verify(struct CalldataSlice input);

#endif
