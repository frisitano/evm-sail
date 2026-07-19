#ifndef BYTE_SLICE_GLUE_H
#define BYTE_SLICE_GLUE_H

#include "quantity_abi.h"
#include "sail_abi.h"

#include <stdbool.h>
#include <stdint.h>

enum evmsail_byte_source_kind {
  EVMSAIL_SOURCE_STATELESS_INPUT = 1,
  EVMSAIL_SOURCE_EVM_MEMORY = 2,
  EVMSAIL_SOURCE_CODE = 4,
  EVMSAIL_SOURCE_LOG_DATA = 6,
  EVMSAIL_SOURCE_OUTPUT = 8,
  EVMSAIL_SOURCE_SCRATCH = 9,
};

/* This header is injected before the generated model defines ByteSlice, so
 * declarations use incomplete generated types. The matching translation unit
 * is compiled against EVMSAIL_MODEL_H. */
struct zByteSlice;

uint64_t evmsail_source_kind(int generated_source);
int evmsail_resolve_byte_source(uint64_t kind, uint64_t off, uint64_t len,
                                const uint8_t **bytes,
                                uint64_t *resolved_len);
#ifdef EVMSAIL_STANDARD_ABI
void stateless_input(struct zByteSlice *out, unit u);
#else
struct zByteSlice stateless_input(unit u);
#endif
const uint8_t *evmsail_stateless_input_ptr(uint64_t off, uint64_t len);
void evmsail_input_reset(void);

uint64_t slice_byte_at(struct zByteSlice slice,
                       EVMSAIL_BYTE_QUANTITY_PARAM(index));
#ifdef EVMSAIL_STANDARD_ABI
void slice_count_nonzero(sail_int *out, struct zByteSlice slice);
#else
uint64_t slice_count_nonzero(struct zByteSlice slice);
#endif
bool slice_strided_zero(struct zByteSlice slice,
                        EVMSAIL_BYTE_QUANTITY_PARAM(start),
                        EVMSAIL_BYTE_QUANTITY_PARAM(stride),
                        EVMSAIL_BYTE_QUANTITY_PARAM(width),
                        EVMSAIL_BYTE_QUANTITY_PARAM(count));
EVMSAIL_WORD_RETURN slice_load_word(EVMSAIL_WORD_RESULT(result)
                                    struct zByteSlice slice,
                                    EVMSAIL_BYTE_QUANTITY_PARAM(index));
EVMSAIL_WORD_RETURN slice_load_n_word(EVMSAIL_WORD_RESULT(result)
                                      struct zByteSlice slice,
                                      EVMSAIL_BYTE_QUANTITY_PARAM(index),
                                      EVMSAIL_BYTE_QUANTITY_PARAM(len));
unit slice_copy_to_memory(struct zByteSlice slice,
                          EVMSAIL_BYTE_QUANTITY_PARAM(dst),
                          EVMSAIL_BYTE_QUANTITY_PARAM(index),
                          EVMSAIL_BYTE_QUANTITY_PARAM(len));

bool scratch_store_bytes(EVMSAIL_BYTE_QUANTITY_PARAM(off),
                         evmsail_byte_list bytes,
                         EVMSAIL_BYTE_QUANTITY_PARAM(len));
bool scratch_store_slice(EVMSAIL_BYTE_QUANTITY_PARAM(off),
                         struct zByteSlice slice);
bool public_output_write(struct zByteSlice output);

bool output_buffer_store(struct zByteSlice slice);

EVMSAIL_HASH_RETURN code_db_store_indexed(EVMSAIL_HASH_RESULT(result)
                                          struct zByteSlice code,
                                          uint64_t jumpdest_ref);

bool accelerator_ripemd160(struct zByteSlice input);
bool accelerator_modexp(struct zByteSlice input,
                        EVMSAIL_BYTE_QUANTITY_PARAM(base_len),
                        EVMSAIL_BYTE_QUANTITY_PARAM(exponent_len),
                        EVMSAIL_BYTE_QUANTITY_PARAM(modulus_len));
bool accelerator_bn254_add(struct zByteSlice input);
bool accelerator_bn254_mul(struct zByteSlice input);
uint64_t accelerator_bn254_pairing(struct zByteSlice input);
bool accelerator_blake2f(struct zByteSlice input, uint64_t rounds,
                         uint64_t final_block);
bool accelerator_kzg_point_evaluation(struct zByteSlice input);
bool accelerator_bls_g1_add(struct zByteSlice input);
bool accelerator_bls_g1_msm(struct zByteSlice input);
bool accelerator_bls_g2_add(struct zByteSlice input);
bool accelerator_bls_g2_msm(struct zByteSlice input);
uint64_t accelerator_bls_pairing(struct zByteSlice input);
bool accelerator_bls_map_fp_to_g1(struct zByteSlice input);
bool accelerator_bls_map_fp2_to_g2(struct zByteSlice input);
bool accelerator_p256_verify(struct zByteSlice input);

#endif
