/* Optimized execution-request root construction. */
#include "evmsail/host/types.h"
#include "evmsail/prelude.h"

#include "evmsail/spec/lib/ssz/stateless_input.h"
#include "lib/mpt/trie.h"
#include "lib/rlp/encoding.h"
#include "evmsail/lib/ssz/stateless_input.h"

#include <stdint.h>

/* Request types are ordered 0..4; empty request lists contribute no digest
 * (EIP-7685). */
bytes32 execution_requests_hash(struct StatelessInputRef input_ref)
{
  const Bytes requests[5] = {
      input_ref.deposits,
      input_ref.withdrawal_requests,
      input_ref.consolidation_requests,
      input_ref.builder_deposit_requests,
      input_ref.builder_exit_requests,
  };
  bytes32 digests[5];
  uint64_t count = 0;
  for (uint8_t type = 0; type < 5; ++type) {
    if (requests[type].len == 0) {
      continue;
    }
    digests[count++] = sha256_request_digest(type, requests[type]);
  }
  return sha256_bytes((const uint8_t *)digests, count * sizeof(*digests));
}

bytes32 transaction_trie_root(struct BoundedSszListRef transactions)
{
  return mpt_transaction_trie_root(transactions);
}

bytes32 withdrawals_trie_root(struct BoundedSszListRef withdrawals)
{
  return mpt_withdrawals_trie_root(withdrawals);
}
