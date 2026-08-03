#ifndef EVMSAIL_OPTIMIZED_EXECUTOR_PAYLOAD_H
#define EVMSAIL_OPTIMIZED_EXECUTOR_PAYLOAD_H

#include "evmsail/prelude.h"

Hash32 execution_requests_hash(
    struct zStatelessInputRef input_ref);
Hash32 transaction_trie_root(
    struct zBoundedSszzListRef transactions);
Hash32 withdrawals_trie_root(
    struct zBoundedSszzListRef withdrawals);

#endif
