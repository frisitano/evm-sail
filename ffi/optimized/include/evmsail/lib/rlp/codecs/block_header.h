#ifndef EVMSAIL_OPTIMIZED_LIB_RLP_CODECS_BLOCK_HEADER_H
#define EVMSAIL_OPTIMIZED_LIB_RLP_CODECS_BLOCK_HEADER_H

#include "evmsail/prelude.h"

Hash32 block_header_hash(
    struct BlockHeader header,
    Hash32 transactions_root,
    Hash32 withdrawals_root,
    Hash32 requests_hash,
    Hash32 block_access_list_hash);

#endif
