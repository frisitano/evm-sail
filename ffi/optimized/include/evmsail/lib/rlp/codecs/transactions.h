#ifndef EVMSAIL_OPTIMIZED_LIB_RLP_CODECS_TRANSACTIONS_H
#define EVMSAIL_OPTIMIZED_LIB_RLP_CODECS_TRANSACTIONS_H

#include "evmsail/prelude.h"

/* Decodes one complete transaction while retaining variable fields as input slices. */
struct zTransactionFields decode_transaction(
    struct zStatelessInputSliceFields transaction,
    struct zStatelessInputSliceFields public_key, uint64_t blob_limit);

#endif
