#ifndef EVMSAIL_OPTIMIZED_LIB_SSZ_STATELESS_INPUT_H
#define EVMSAIL_OPTIMIZED_LIB_SSZ_STATELESS_INPUT_H

#include "evmsail/prelude.h"

struct zStatelessInputRef decode_stateless_input_ref(
    struct zStatelessInputSliceFields input);

struct zWithdrawal decode_withdrawal(
    struct zStatelessInputSliceFields withdrawal);

U256 ssz_u256(struct zStatelessInputSliceFields input,
                           uint64_t offset);

Hash32 sha256_request_digest(
    uint64_t request_type, struct zStatelessInputSliceFields request);

unit index_witness_nodes(struct zBoundedSszzListRef nodes);
unit index_witness_codes(struct zBoundedSszzListRef codes,
                                 bool amsterdam_or_later);

#endif
