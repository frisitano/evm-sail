#pragma once

#include "evmsail/spec/lib/rlp/encoding.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

uint64_t rlp_uint64_append(uint64_t prefix, uint64_t next);

struct tuple_bool_uint_32_uint_64 rlp_ref_hdr(Bytes b);

Bytes rlp_decode_list(struct RlpFieldRef f);

struct RlpFieldRef rlp_decode_item(Bytes cursor);

Bytes rlp_cursor_advance(Bytes cursor, uint32_t consumed);

void rlp_cursor_expect_end(Bytes cursor);

struct RlpFieldRef rlp_single_ref(Bytes item);

Bytes rlp_node_cursor(Bytes node);

Bytes rlp_item_content(struct RlpFieldRef f);

struct tuple_bool_uint_32_uint_64 scratch_rlp_ref_hdr(Bytes b);

Bytes scratch_rlp_decode_list(struct ScratchRlpFieldRef f);

struct ScratchRlpFieldRef scratch_rlp_decode_item(Bytes cursor);

Bytes scratch_rlp_cursor_advance(Bytes cursor, uint32_t consumed);

void scratch_rlp_cursor_expect_end(Bytes cursor);

struct ScratchRlpFieldRef scratch_rlp_single_ref(Bytes item);

Bytes scratch_rlp_node_cursor(Bytes node);

Bytes scratch_rlp_item_content(struct ScratchRlpFieldRef f);

u256 scratch_rlp_decode_word(struct ScratchRlpFieldRef f);

bool rlp_ref_framing_canonical(struct RlpFieldRef f);

bool rlp_ref_bytes_canonical(struct RlpFieldRef f);

bool rlp_item_uint_canonical(struct RlpFieldRef f);

u256 rlp_decode_word(struct RlpFieldRef f);

u256 rlp_decode_u256(struct RlpFieldRef f);

uint64_t rlp_decode_uint64(struct RlpFieldRef f);

uint64_t rlp_uint64_width_Bytes_uint8_t_to_uint64_t(Bytes content, uint8_t width);

uint64_t rlp_uint64_width_Bytes_uint8_t_to_uint64_t_variant_2(Bytes content, uint8_t width);

uint64_t scratch_rlp_uint64_width_Bytes_uint8_t_to_uint64_t(Bytes content, uint8_t width);

uint64_t scratch_rlp_uint64_width_Bytes_uint8_t_to_uint64_t_variant_2(Bytes content, uint8_t width);


#ifdef __cplusplus
}
#endif
