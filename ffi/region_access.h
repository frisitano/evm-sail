#ifndef EVMSAIL_REGION_ACCESS_H
#define EVMSAIL_REGION_ACCESS_H

#include "quantity_abi.h"
#include "sail_abi.h"

#include <stdbool.h>
#include <stdint.h>

/*
 * The generated model owns all aggregate layouts.  Each nominal slice carries
 * only {off, len}; its C function selects the backing region statically.
 */
struct zStatelessInputSliceFields;
struct zScratchSliceFields;
struct zMemorySliceFields;
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

#ifdef EVMSAIL_STANDARD_ABI
void stateless_input(struct zStatelessInputSliceFields *out, unit u);
void mem_expand(struct zMemorySliceFields *out,
                EVMSAIL_BYTE_QUANTITY_PARAM(len));
void nodedb_lookup(struct zStatelessInputSliceFields *out, sail_hash hash);
#else
struct zStatelessInputSliceFields stateless_input(unit u);
struct zMemorySliceFields mem_expand(EVMSAIL_BYTE_QUANTITY_PARAM(len));
struct zStatelessInputSliceFields nodedb_lookup(sail_hash hash);
#endif

#ifndef EVMSAIL_STANDARD_ABI
struct zOutputSliceFields evmsail_output_buffer_slice(uint64_t len);
#endif

uint64_t stateless_input_byte_at(
    struct zStatelessInputSliceFields slice,
    EVMSAIL_BYTE_QUANTITY_PARAM(index));
uint64_t memory_slice_byte_at(struct zMemorySliceFields slice,
                              EVMSAIL_BYTE_QUANTITY_PARAM(index));
uint64_t code_region_byte_at(struct zCodeRegionSliceFields slice,
                             EVMSAIL_BYTE_QUANTITY_PARAM(index));
uint64_t scratch_slice_byte_at(struct zScratchSliceFields slice,
                               EVMSAIL_BYTE_QUANTITY_PARAM(index));
uint64_t log_data_slice_byte_at(struct zLogDataSliceFields slice,
                                EVMSAIL_BYTE_QUANTITY_PARAM(index));
uint64_t output_slice_byte_at(struct zOutputSliceFields slice,
                              EVMSAIL_BYTE_QUANTITY_PARAM(index));

bool scratch_input_slices_equal(struct zScratchSliceFields left,
                                struct zStatelessInputSliceFields right);
bool log_input_slices_equal(struct zLogDataSliceFields left,
                            struct zStatelessInputSliceFields right);
bool input_code_slices_equal(struct zStatelessInputSliceFields left,
                             struct zCodeRegionSliceFields right);
bool evmsail_logs_bloom_matches_ref(
    sail_logs_bloom computed, struct zStatelessInputSliceFields reference);

#ifdef EVMSAIL_STANDARD_ABI
void stateless_input_count_nonzero(
    sail_int *out, struct zStatelessInputSliceFields slice);
#else
uint64_t stateless_input_count_nonzero(
    struct zStatelessInputSliceFields slice);
#endif

bool stateless_input_strided_zero(
    struct zStatelessInputSliceFields slice,
    EVMSAIL_BYTE_QUANTITY_PARAM(start),
    EVMSAIL_BYTE_QUANTITY_PARAM(stride),
    EVMSAIL_BYTE_QUANTITY_PARAM(width),
    EVMSAIL_BYTE_QUANTITY_PARAM(count));
bool memory_slice_strided_zero(
    struct zMemorySliceFields slice, EVMSAIL_BYTE_QUANTITY_PARAM(start),
    EVMSAIL_BYTE_QUANTITY_PARAM(stride),
    EVMSAIL_BYTE_QUANTITY_PARAM(width),
    EVMSAIL_BYTE_QUANTITY_PARAM(count));

#define EVMSAIL_DECLARE_SLICE_LOAD(prefix, type)                              \
  EVMSAIL_WORD_RETURN prefix##_load_word(                                     \
      EVMSAIL_WORD_RESULT(result) struct type slice,                           \
      EVMSAIL_BYTE_QUANTITY_PARAM(index))

#define EVMSAIL_DECLARE_SLICE_LOAD_N(prefix, type)                            \
  EVMSAIL_WORD_RETURN prefix##_load_n_word(                                   \
      EVMSAIL_WORD_RESULT(result) struct type slice,                           \
      EVMSAIL_BYTE_QUANTITY_PARAM(index),                                      \
      EVMSAIL_BYTE_QUANTITY_PARAM(len))

#define EVMSAIL_DECLARE_SLICE_COPY(prefix, type)                              \
  unit prefix##_copy_to_memory(struct type slice,                             \
                               EVMSAIL_BYTE_QUANTITY_PARAM(dst),               \
                               EVMSAIL_BYTE_QUANTITY_PARAM(index),             \
                               EVMSAIL_BYTE_QUANTITY_PARAM(len))

EVMSAIL_DECLARE_SLICE_LOAD(stateless_input, zStatelessInputSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD(memory_slice, zMemorySliceFields);
EVMSAIL_DECLARE_SLICE_LOAD(code_region, zCodeRegionSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD(scratch_slice, zScratchSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD(log_data_slice, zLogDataSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD(output_slice, zOutputSliceFields);

EVMSAIL_DECLARE_SLICE_LOAD_N(stateless_input, zStatelessInputSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD_N(code_region, zCodeRegionSliceFields);
EVMSAIL_DECLARE_SLICE_LOAD_N(scratch_slice, zScratchSliceFields);

EVMSAIL_DECLARE_SLICE_COPY(stateless_input, zStatelessInputSliceFields);
EVMSAIL_DECLARE_SLICE_COPY(memory_slice, zMemorySliceFields);
EVMSAIL_DECLARE_SLICE_COPY(code_region, zCodeRegionSliceFields);
EVMSAIL_DECLARE_SLICE_COPY(output_slice, zOutputSliceFields);

#undef EVMSAIL_DECLARE_SLICE_LOAD
#undef EVMSAIL_DECLARE_SLICE_LOAD_N
#undef EVMSAIL_DECLARE_SLICE_COPY

void scratch_store_byte(struct zScratchRegionResult *result,
                        EVMSAIL_BYTE_QUANTITY_PARAM(off), uint64_t data);
void scratch_store_stateless_input(
    struct zScratchRegionResult *result,
    EVMSAIL_BYTE_QUANTITY_PARAM(off),
    struct zStatelessInputSliceFields slice);
void scratch_store_scratch(struct zScratchRegionResult *result,
                           EVMSAIL_BYTE_QUANTITY_PARAM(off),
                           struct zScratchSliceFields slice);
void scratch_store_log_data(struct zScratchRegionResult *result,
                            EVMSAIL_BYTE_QUANTITY_PARAM(off),
                            struct zLogDataSliceFields slice);
void scratch_store_output(struct zScratchRegionResult *result,
                          EVMSAIL_BYTE_QUANTITY_PARAM(off),
                          struct zOutputSliceFields slice);
void scratch_store_address(struct zScratchRegionResult *result,
                           EVMSAIL_BYTE_QUANTITY_PARAM(off),
                           sail_address data);
void scratch_store_b256(struct zScratchRegionResult *result,
                        EVMSAIL_BYTE_QUANTITY_PARAM(off), sail_b256 data,
                        uint64_t len);
void scratch_store_fixed_bytes_256(struct zScratchRegionResult *result,
                                   EVMSAIL_BYTE_QUANTITY_PARAM(off),
                                   sail_fixed_bytes_256 data);
void scratch_store_receipt_logs_bloom(
    struct zScratchRegionResult *result,
    EVMSAIL_BYTE_QUANTITY_PARAM(off), uint64_t start, uint64_t count);
void scratch_store_word(struct zScratchRegionResult *result,
                        EVMSAIL_BYTE_QUANTITY_PARAM(off),
                        EVMSAIL_WORD_PARAM(data), uint64_t len);

bool public_output_write(struct zScratchSliceFields output);
bool output_buffer_store_memory(struct zMemorySliceFields slice);
bool output_buffer_store_input(struct zStatelessInputSliceFields slice);

EVMSAIL_HASH_RETURN code_db_store_indexed(
    EVMSAIL_HASH_RESULT(result) struct zCodeRegionSliceFields code,
    uint64_t jumpdest_ref);

bool accelerator_ripemd160(struct zCalldataSlice input);
bool accelerator_modexp(struct zCalldataSlice input,
                        EVMSAIL_BYTE_QUANTITY_PARAM(base_len),
                        EVMSAIL_BYTE_QUANTITY_PARAM(exponent_len),
                        EVMSAIL_BYTE_QUANTITY_PARAM(modulus_len));
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
