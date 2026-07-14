/* ByteSlice is a generated Sail aggregate. Keep its exact layout at this one
 * model-aware boundary and pass the complete value from Sail; scalar C
 * mechanisms continue to consume the host source kind and span internally. */
#include EVMSAIL_MODEL_H

#include "code_db.h"
#include "host_crypto.h"
#include "lbits_convert.h"
#include "memory.h"
#include "precompiles.h"
#include "output.h"
#include "scratch.h"

static uint64_t source_kind(enum zByteSource source) {
  switch (source) {
    case zStatelessInputSource:
      return EVMSAIL_SOURCE_STATELESS_INPUT;
    case zMemorySource:
      return EVMSAIL_SOURCE_MEMORY;
    case zCodeSource:
      return EVMSAIL_SOURCE_CODE;
    case zLogDataSource:
      return EVMSAIL_SOURCE_LOG_DATA;
    case zMemoryArenaSource:
      return EVMSAIL_SOURCE_MEMORY_ARENA;
    case zOutputSource:
      return EVMSAIL_SOURCE_OUTPUT;
    case zScratchSource:
      return EVMSAIL_SOURCE_SCRATCH;
  }
  return 0;
}

bool scratch_store_bytes(uint64_t off,
                         struct node_zz5listz8z5bvz9 *bytes, uint64_t len) {
  uint8_t *out = scratch_prepare(off, len);
  if (len != 0 && !out) return false;

  struct node_zz5listz8z5bvz9 *byte = bytes;
  for (uint64_t i = 0; i < len; i++) {
    if (!byte) return false;
    lbits_to_be_bytes(out + i, 1, byte->hd);
    byte = byte->tl;
  }
  if (byte) return false;
  return scratch_commit(off, len);
}

bool scratch_store_slice(uint64_t off, struct zByteSlice slice) {
  return scratch_append_source(off, source_kind(slice.zsource), slice.zoff,
                               slice.zlen);
}

uint64_t slice_byte_at(struct zByteSlice slice, uint64_t index) {
  return slice_byte_at_source(source_kind(slice.zsource), slice.zoff,
                              slice.zlen, index);
}

uint64_t slice_count_nonzero(struct zByteSlice slice) {
  return slice_count_nonzero_source(source_kind(slice.zsource), slice.zoff,
                                    slice.zlen);
}

bool slice_strided_zero(struct zByteSlice slice, uint64_t start,
                        uint64_t stride, uint64_t width, uint64_t count) {
  return slice_strided_zero_source(source_kind(slice.zsource), slice.zoff,
                                   slice.zlen, start, stride, width, count);
}

void slice_load_word(lbits *result, struct zByteSlice slice, uint64_t index) {
  slice_load_word_source(result, source_kind(slice.zsource), slice.zoff,
                         slice.zlen, index);
}

void slice_load_n_word(lbits *result, struct zByteSlice slice, uint64_t index,
                       uint64_t len) {
  slice_load_n_word_source(result, source_kind(slice.zsource), slice.zoff,
                           slice.zlen, index, len);
}

unit slice_copy_to_memory(struct zByteSlice slice, uint64_t dst,
                          uint64_t index, uint64_t len) {
  return slice_copy_to_memory_source(source_kind(slice.zsource), slice.zoff,
                                     slice.zlen, dst, index, len);
}

bool output_buffer_store(struct zByteSlice slice) {
  return output_buffer_store_source(source_kind(slice.zsource), slice.zoff,
                                    slice.zlen);
}

void code_db_store_indexed(lbits *result, struct zByteSlice code,
                           struct node_zz5listz8z5bvz9 *jumpdests) {
  code_db_store_indexed_source(result, source_kind(code.zsource), code.zoff,
                               code.zlen, jumpdests);
}

bool accelerator_ripemd160(struct zByteSlice input) {
  return accelerator_ripemd160_source(source_kind(input.zsource), input.zoff,
                                      input.zlen);
}

bool accelerator_modexp(struct zByteSlice input, uint64_t base_len,
                        uint64_t exponent_len, uint64_t modulus_len) {
  return accelerator_modexp_source(source_kind(input.zsource), input.zoff,
                                   input.zlen, base_len, exponent_len,
                                   modulus_len);
}

bool accelerator_bn254_add(struct zByteSlice input) {
  return accelerator_bn254_add_source(source_kind(input.zsource), input.zoff,
                                      input.zlen);
}

bool accelerator_bn254_mul(struct zByteSlice input) {
  return accelerator_bn254_mul_source(source_kind(input.zsource), input.zoff,
                                      input.zlen);
}

uint64_t accelerator_bn254_pairing(struct zByteSlice input) {
  return accelerator_bn254_pairing_source(
      source_kind(input.zsource), input.zoff, input.zlen);
}

bool accelerator_blake2f(struct zByteSlice input, uint64_t rounds,
                         uint64_t final_block) {
  return accelerator_blake2f_source(source_kind(input.zsource), input.zoff,
                                    input.zlen, rounds, final_block);
}

bool accelerator_kzg_point_evaluation(struct zByteSlice input) {
  return accelerator_kzg_point_evaluation_source(
      source_kind(input.zsource), input.zoff, input.zlen);
}

bool accelerator_bls_g1_add(struct zByteSlice input) {
  return accelerator_bls_g1_add_source(source_kind(input.zsource), input.zoff,
                                       input.zlen);
}

bool accelerator_bls_g1_msm(struct zByteSlice input) {
  return accelerator_bls_g1_msm_source(source_kind(input.zsource), input.zoff,
                                       input.zlen);
}

bool accelerator_bls_g2_add(struct zByteSlice input) {
  return accelerator_bls_g2_add_source(source_kind(input.zsource), input.zoff,
                                       input.zlen);
}

bool accelerator_bls_g2_msm(struct zByteSlice input) {
  return accelerator_bls_g2_msm_source(source_kind(input.zsource), input.zoff,
                                       input.zlen);
}

uint64_t accelerator_bls_pairing(struct zByteSlice input) {
  return accelerator_bls_pairing_source(source_kind(input.zsource), input.zoff,
                                        input.zlen);
}

bool accelerator_bls_map_fp_to_g1(struct zByteSlice input) {
  return accelerator_bls_map_fp_to_g1_source(
      source_kind(input.zsource), input.zoff, input.zlen);
}

bool accelerator_bls_map_fp2_to_g2(struct zByteSlice input) {
  return accelerator_bls_map_fp2_to_g2_source(
      source_kind(input.zsource), input.zoff, input.zlen);
}

bool accelerator_p256_verify(struct zByteSlice input) {
  return accelerator_p256_verify_source(source_kind(input.zsource), input.zoff,
                                        input.zlen);
}
