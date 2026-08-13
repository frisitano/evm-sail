#pragma once

#include "evmsail/spec/lib/rlp/codecs/block_header.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

bytes32 transaction_trie_root_(struct BoundedSszListRef txs);

bytes32 withdrawals_trie_root_(struct BoundedSszListRef wds);

uint32_t expected_payload_excess_blob_gas(struct WitnessContext witness);

bytes32 execution_requests_hash_(struct StatelessInputRef input_ref);

void validate_execution_payload(struct StatelessInput input, struct StatelessInputRef input_ref, struct WitnessContext witness);


#ifdef __cplusplus
}
#endif
