#pragma once

#include "evmsail/spec/lib/rlp/codecs/block_access_list.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

bool historical_schema_has_slot(Bytes cursor, u256 slot);

bool historical_schema_has_access(Bytes cursor, u256 address,
                                  struct option_R__sail_c_repr_u256_ slot);

void validate_historical_schema_slots(Bytes cursor, u256 previous, bool have_previous,
                                      bytes32 storage_root);

void validate_historical_schema_accounts(Bytes cursor, u256 previous, bool have_previous);

void validate_historical_access_schema(Bytes bytes);

void validate_block_access_list_(Bytes bytes, uint64_t block_gas_limit);

void bal_compare_index_word(struct RlpFieldRef pair, uint8_t index, u256 value);

#ifdef __cplusplus
}
#endif
