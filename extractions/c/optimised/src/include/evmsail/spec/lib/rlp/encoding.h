#pragma once

#include "evmsail/spec/lib/ssz/ssz.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct RlpEncoder
struct RlpEncoder {
  uint32_t expected_len;
  uint32_t start;
};

uint32_t rlp_scratch_small_length(uint8_t value);

uint64_t rlp_nat_length_byte(uint8_t value);

uint8_t rlp_minimal_word_len(u256 w);

uint32_t rlp_materialized_slice_size(uint32_t length_, uint64_t first);

uint32_t rlp_scratch_region_size(Bytes data);

uint32_t rlp_log_data_size(Bytes data);

uint8_t rlp_uint_word_size(u256 w);

uint32_t rlp_scratch_list_size(uint32_t content_len);

uint32_t rlp_input_scratch_slice_size(Bytes data);

uint32_t rlp_scratch_scratch_slice_size(Bytes data);

uint32_t rlp_log_scratch_slice_size(Bytes data);

void rlp_write_input_slice(Bytes data);

void rlp_write_scratch_slice(Bytes data);

void rlp_write_log_data_slice(Bytes data);

void rlp_write_uint_word(u256 w);

void rlp_write_word(u256 w);

void rlp_write_addr(bytes20 a);

struct RlpEncoder rlp_encoder_begin(uint32_t expected_len);

Bytes rlp_encoder_finish(struct RlpEncoder encoder);

void rlp_encoder_rewind(struct RlpEncoder encoder);

struct RlpEncoder rlp_encoder_begin_uint8_t_to_struct_RlpEncoder(uint8_t expected_len);

uint8_t rlp_length_prefix_len_uint16_t_to_uint8_t(uint16_t len);

uint8_t rlp_length_prefix_len_uint32_t_to_uint8_t(uint32_t len);

uint8_t rlp_length_prefix_len_uint8_t_to_uint8_t(void);

uint8_t rlp_length_prefix_len_uint8_t_to_uint8_t_variant_2(uint8_t len);

uint8_t rlp_length_prefix_len_uint8_t_to_uint8_t_variant_3(uint8_t len);

u256 rlp_length_word_uint16_t_to_u256(uint16_t value);

u256 rlp_length_word_uint32_t_to_u256(uint32_t value);

u256 rlp_length_word_uint8_t_to_u256(uint8_t value);

uint8_t rlp_list_size_uint8_t_to_uint8_t(uint8_t content_len);

uint8_t rlp_list_size_uint8_t_to_uint8_t_variant_2(uint8_t content_len);

uint8_t rlp_list_size_uint8_t_to_uint8_t_variant_3(uint8_t content_len);

uint8_t rlp_minimal_word_len_u128_to_uint8_t(u128 w);

uint8_t rlp_minimal_word_len_uint64_t_to_uint8_t(uint64_t w);

uint8_t rlp_minimal_word_len_uint8_t_to_uint8_t(uint8_t w);

uint32_t rlp_scratch_length_add_uint16_t_uint32_t_to_uint32_t(uint16_t left, uint32_t right);

uint32_t rlp_scratch_length_add_uint16_t_uint8_t_to_uint32_t(uint16_t left, uint8_t right);

uint32_t rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t(uint32_t left, uint32_t right);

uint32_t rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t_variant_2(uint32_t left, uint32_t right);

uint32_t rlp_scratch_length_add_uint32_t_uint32_t_to_uint32_t_variant_3(uint32_t left, uint32_t right);

uint32_t rlp_scratch_length_add_uint32_t_uint8_t_to_uint32_t(uint32_t left, uint8_t right);

uint32_t rlp_scratch_length_add_uint8_t_uint32_t_to_uint32_t(uint8_t left, uint32_t right);

uint8_t rlp_uint_word_size_uint64_t_to_uint8_t(uint64_t w);

uint8_t rlp_uint_word_size_uint8_t_to_uint8_t(uint8_t w);

void rlp_write_list_prefix_uint16_t_to_unit(uint16_t content_len);

void rlp_write_list_prefix_uint32_t_to_unit(uint32_t content_len);

void rlp_write_list_prefix_uint8_t_to_unit(uint8_t content_len);

void rlp_write_list_prefix_uint8_t_to_unit_variant_2(uint8_t content_len);

void rlp_write_list_prefix_uint8_t_to_unit_variant_3(uint8_t content_len);

void rlp_write_string_prefix_uint16_t_uint64_t_to_unit(uint16_t len);

void rlp_write_string_prefix_uint32_t_uint64_t_to_unit(uint32_t len, uint64_t first);

void rlp_write_string_prefix_uint8_t_uint64_t_to_unit(uint8_t len);

void rlp_write_string_prefix_uint8_t_uint64_t_to_unit_variant_2(uint8_t len, uint64_t first);

void rlp_write_uint_word_u128_to_unit(u128 w);

void rlp_write_uint_word_uint64_t_to_unit(uint64_t w);

void rlp_write_uint_word_uint8_t_to_unit(uint8_t w);

extern const uint8_t RLP_SHORT_LENGTH_LIMIT;


extern const uint8_t RLP_WORD_LENGTH_LIMIT;


extern const uint8_t RLP_UINT64_LENGTH_LIMIT;



#ifdef __cplusplus
}
#endif
