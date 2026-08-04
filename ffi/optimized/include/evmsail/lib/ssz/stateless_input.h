#ifndef EVMSAIL_OPTIMIZED_LIB_SSZ_STATELESS_INPUT_H
#define EVMSAIL_OPTIMIZED_LIB_SSZ_STATELESS_INPUT_H

#include "evmsail/prelude.h"

struct StatelessInputRef decode_stateless_input_ref(
    struct StatelessInputSliceFields input);

struct Withdrawal decode_withdrawal(
    struct StatelessInputSliceFields withdrawal);

U256 ssz_u256(struct StatelessInputSliceFields input,
                   uint32_t offset);

Hash32 sha256_request_digest(
    uint64_t request_type, struct StatelessInputSliceFields request);

unit index_witness_nodes(struct BoundedSszListRef nodes);
unit index_witness_codes(struct BoundedSszListRef codes,
                                 bool amsterdam_or_later);

#endif
