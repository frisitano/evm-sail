/* Optimized execution-request root construction. */
#include "evmsail/prelude.h"

#include "evmsail/executor/payload.h"
#include "lib/mpt/trie.h"
#include "lib/rlp/encoding.h"
#include "evmsail/lib/ssz/stateless_input.h"

#include <stdint.h>

static bool request_digest(uint8_t type,
                           struct zStatelessInputSliceFields request,
                           Hash32 *digest) {
  const uint64_t len = request.zlen;
  if (len == 0)
    return false;
  *digest = sha256_request_digest(type, request);
  return true;
}

Hash32 execution_requests_hash(
    struct zStatelessInputRef input_ref) {
  Hash32 digests[5];
  uint64_t count = 0;
  if (request_digest(0, input_ref.zdeposits, &digests[count]))
    ++count;
  if (request_digest(1, input_ref.zwithdrawal_requests, &digests[count]))
    ++count;
  if (request_digest(2, input_ref.zconsolidation_requests, &digests[count]))
    ++count;
  if (request_digest(3, input_ref.zbuilder_deposit_requests, &digests[count]))
    ++count;
  if (request_digest(4, input_ref.zbuilder_exit_requests, &digests[count]))
    ++count;
  return sha256_bytes((const uint8_t *)digests,
                                        count * sizeof(*digests));
}

Hash32 transaction_trie_root(
    struct zBoundedSszzListRef transactions) {
  return mpt_transaction_trie_root(transactions);
}

Hash32 withdrawals_trie_root(
    struct zBoundedSszzListRef withdrawals) {
  return mpt_withdrawals_trie_root(withdrawals);
}
