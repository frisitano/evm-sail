#pragma once

#include "evmsail/spec/evm/transaction.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct TriePath
struct TriePath {
  bytes32 data;
  uint8_t len;
};

// struct tuple_bool_TriePath
struct tuple_bool_TriePath {
  bool tup0;
  struct TriePath tup1;
};

struct TriePath path_new(bytes32 data, uint8_t len);

struct tuple_bool_TriePath hex_prefix_decode_ref(struct RlpFieldRef f);

uint8_t path_len(struct TriePath path);

uint64_t path_nibble(struct TriePath path, uint8_t i);

struct TriePath path_append_nibble(struct TriePath path, uint64_t value);

struct TriePath path_single(uint64_t n);

struct TriePath path_concat(struct TriePath a, struct TriePath b);

struct TriePath path_drop(struct TriePath path, uint8_t n);

bool path_eq(struct TriePath a, struct TriePath b);

bool path_prefix_of(struct TriePath prefix, struct TriePath path);

uint8_t common_prefix_length(struct TriePath a, struct TriePath b);

uint8_t hex_prefix_encoded_length(struct TriePath path);

uint64_t hex_prefix_first_byte(struct TriePath path, bool is_leaf);

struct tuple_bool_TriePath scratch_hex_prefix_decode_ref(struct ScratchRlpFieldRef f);

uint8_t path_byte_index(uint8_t i);

struct TriePath path_drop_struct_TriePath_uint8_t_to_struct_TriePath(struct TriePath path, uint8_t n);

struct TriePath path_drop_struct_TriePath_uint8_t_to_struct_TriePath_variant_2(struct TriePath path, uint8_t n);

bool path_matches(struct TriePath key, uint8_t pos, struct TriePath seg);

uint64_t path_nibble_struct_TriePath_uint8_t_to_uint64_t(struct TriePath path, uint8_t i);

extern const uint8_t HEX_PREFIX_MAX_LENGTH;



#ifdef __cplusplus
}
#endif
