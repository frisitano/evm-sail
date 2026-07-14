#ifndef BYTE_SLICE_GLUE_H
#define BYTE_SLICE_GLUE_H

#include "sail.h"

#include <stdbool.h>
#include <stdint.h>

/* This header is injected before the generated model defines ByteSlice and
 * JumpdestBitmap, so declarations use incomplete generated types. The matching
 * translation unit is compiled against EVMSAIL_MODEL_H. */
struct zByteSlice;
struct node_zz5listz8z5bvz9;

uint64_t slice_byte_at(struct zByteSlice slice, uint64_t index);
uint64_t slice_count_nonzero(struct zByteSlice slice);
bool slice_strided_zero(struct zByteSlice slice, uint64_t start,
                        uint64_t stride, uint64_t width, uint64_t count);
void slice_load_word(lbits *result, struct zByteSlice slice, uint64_t index);
void slice_load_n_word(lbits *result, struct zByteSlice slice, uint64_t index,
                       uint64_t len);
unit slice_copy_to_memory(struct zByteSlice slice, uint64_t dst,
                          uint64_t index, uint64_t len);

bool scratch_store_bytes(uint64_t off,
                         struct node_zz5listz8z5bvz9 *bytes, uint64_t len);
bool scratch_store_slice(uint64_t off, struct zByteSlice slice);

bool output_buffer_store(struct zByteSlice slice);

void code_db_store_indexed(lbits *result, struct zByteSlice code,
                           struct node_zz5listz8z5bvz9 *jumpdests);

bool accelerator_ripemd160(struct zByteSlice input);
bool accelerator_modexp(struct zByteSlice input, uint64_t base_len,
                        uint64_t exponent_len, uint64_t modulus_len);
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
