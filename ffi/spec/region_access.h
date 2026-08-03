#ifndef EVMSAIL_REGION_ACCESS_H
#define EVMSAIL_REGION_ACCESS_H

#ifndef EVMSAIL_MODEL_H
#error "EVMSAIL_MODEL_H must name the generated model header"
#endif
#include EVMSAIL_MODEL_H

#include <stdbool.h>
#include <stdint.h>

/*
 * The generated model owns all aggregate layouts.  Each nominal slice carries
 * only {off, len}; its C function selects the backing region statically.
 */
struct zStatelessInputSliceFields;
struct zScratchSliceFields;
struct zEvmMemorySliceFields;
struct zCodeRegionSliceFields;
struct zLogDataSliceFields;
struct zOutputSliceFields;
struct zScratchRegionResult;
struct zCalldataSlice;

bool evmsail_stateless_input_contains(uint64_t pointer, uint64_t len);
const uint8_t *evmsail_stateless_input_ptr(uint64_t off, uint64_t len);
const uint8_t *evmsail_memory_ptr(uint64_t off, uint64_t len);
const uint8_t *evmsail_code_ptr(uint64_t off, uint64_t len);
const uint8_t *evmsail_scratch_ptr(uint64_t off, uint64_t len);
const uint8_t *evmsail_log_data_ptr(uint64_t off, uint64_t len);
const uint8_t *evmsail_output_ptr(uint64_t off, uint64_t len);
void evmsail_input_reset(void);

struct zStatelessInputSliceFields stateless_input(unit u);
struct zEvmMemorySliceFields mem_expand(uint64_t len);
struct zStatelessInputSliceFields nodedb_lookup(sail_fixed_bytes_32 hash);


uint64_t stateless_input_byte_at(
    struct zStatelessInputSliceFields slice, uint64_t index);
uint64_t memory_slice_byte_at(struct zEvmMemorySliceFields slice,
                              uint64_t index);
uint64_t code_region_byte_at(struct zCodeRegionSliceFields slice,
                             uint64_t index);
uint64_t scratch_slice_byte_at(struct zScratchSliceFields slice,
                               uint64_t index);
uint64_t log_data_slice_byte_at(struct zLogDataSliceFields slice,
                                uint64_t index);
uint64_t output_slice_byte_at(struct zOutputSliceFields slice,
                              uint64_t index);

bool scratch_input_slices_equal(struct zScratchSliceFields left,
                                struct zStatelessInputSliceFields right);
bool log_input_slices_equal(struct zLogDataSliceFields left,
                            struct zStatelessInputSliceFields right);
bool input_code_slices_equal(struct zStatelessInputSliceFields left,
                             struct zCodeRegionSliceFields right);
bool evmsail_logs_bloom_matches_ref(
    sail_fixed_bytes_256 computed, struct zStatelessInputSliceFields reference);

uint64_t stateless_input_count_nonzero(
    struct zStatelessInputSliceFields slice);

bool stateless_input_strided_zero(
    struct zStatelessInputSliceFields slice, uint64_t start, uint64_t stride,
    uint64_t width, uint64_t count);
bool memory_slice_strided_zero(
    struct zEvmMemorySliceFields slice, uint64_t start, uint64_t stride,
    uint64_t width, uint64_t count);

#define EVMSAIL_DECLARE_SLICE_LOAD(prefix, type)                              \
  sail_u256 prefix##_load_word(                                     \
       struct type slice,                           \
      uint64_t index)

#define EVMSAIL_DECLARE_SLICE_LOAD_N(prefix, type)                            \
  sail_u256 prefix##_load_n_word(                                   \
       struct type slice,                           \
      uint64_t index, uint64_t len)

#define EVMSAIL_DECLARE_SLICE_COPY(prefix, type)                              \
  unit prefix##_copy_to_memory(struct type slice,                             \
                               uint64_t dst, uint64_t index, uint64_t len)

EVMSAIL_DECLARE_SLICE_LOAD(stateless_input, zStatelessInputSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD(memory_slice, zEvmMemorySliceFields);
EVMSAIL_DECLARE_SLICE_LOAD(code_region, zCodeRegionSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD(scratch_slice, zScratchSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD(log_data_slice, zLogDataSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD(output_slice, zOutputSliceFields);

EVMSAIL_DECLARE_SLICE_LOAD_N(stateless_input, zStatelessInputSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD_N(code_region, zCodeRegionSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD_N(scratch_slice, zScratchSliceFields);

EVMSAIL_DECLARE_SLICE_COPY(stateless_input, zStatelessInputSliceFields);
EVMSAIL_DECLARE_SLICE_COPY(memory_slice, zEvmMemorySliceFields);
EVMSAIL_DECLARE_SLICE_COPY(code_region, zCodeRegionSliceFields);
EVMSAIL_DECLARE_SLICE_COPY(output_slice, zOutputSliceFields);

#undef EVMSAIL_DECLARE_SLICE_LOAD
#undef EVMSAIL_DECLARE_SLICE_LOAD_N
#undef EVMSAIL_DECLARE_SLICE_COPY

void scratch_store_byte(struct zScratchRegionResult *result,
                        uint64_t off, uint64_t data);
void scratch_store_stateless_input(
    struct zScratchRegionResult *result, uint64_t off,
    struct zStatelessInputSliceFields slice);
void scratch_store_scratch(struct zScratchRegionResult *result,
                           uint64_t off,
                           struct zScratchSliceFields slice);
void scratch_store_log_data(struct zScratchRegionResult *result,
                            uint64_t off,
                            struct zLogDataSliceFields slice);
void scratch_store_output(struct zScratchRegionResult *result,
                          uint64_t off,
                          struct zOutputSliceFields slice);
void scratch_store_address(struct zScratchRegionResult *result,
                           uint64_t off,
                           sail_fixed_bytes_20 data);
void scratch_store_b256(struct zScratchRegionResult *result,
                        uint64_t off, sail_fixed_bytes_32 data,
                        uint64_t len);
void scratch_store_fixed_bytes_256(struct zScratchRegionResult *result,
                                   uint64_t off,
                                   sail_fixed_bytes_256 data);
void scratch_store_receipt_logs_bloom(
    struct zScratchRegionResult *result, uint64_t off, uint64_t start,
    uint64_t count);
void scratch_store_word(struct zScratchRegionResult *result,
                        uint64_t off,
                        const sail_u256 data, uint64_t len);

bool public_output_write(struct zScratchSliceFields output);
bool output_buffer_store_memory(struct zEvmMemorySliceFields slice);
bool output_buffer_store_input(struct zStatelessInputSliceFields slice);

sail_fixed_bytes_32 code_db_store_indexed(
    struct zCodeRegionSliceFields code, uint64_t jumpdest_ref);

bool accelerator_ripemd160(struct zCalldataSlice input);
bool accelerator_modexp(struct zCalldataSlice input, uint64_t base_len,
                        uint64_t exponent_len, uint64_t modulus_len);
bool accelerator_bn254_add(struct zCalldataSlice input);
bool accelerator_bn254_mul(struct zCalldataSlice input);
uint64_t accelerator_bn254_pairing(struct zCalldataSlice input);
bool accelerator_blake2f(struct zCalldataSlice input, uint64_t rounds,
                         uint64_t final_block);
bool accelerator_kzg_point_evaluation(struct zCalldataSlice input);
bool accelerator_bls_g1_add(struct zCalldataSlice input);
bool accelerator_bls_g1_msm(struct zCalldataSlice input);
bool accelerator_bls_g2_add(struct zCalldataSlice input);
bool accelerator_bls_g2_msm(struct zCalldataSlice input);
uint64_t accelerator_bls_pairing(struct zCalldataSlice input);
bool accelerator_bls_map_fp_to_g1(struct zCalldataSlice input);
bool accelerator_bls_map_fp2_to_g2(struct zCalldataSlice input);
bool accelerator_p256_verify(struct zCalldataSlice input);

#endif
