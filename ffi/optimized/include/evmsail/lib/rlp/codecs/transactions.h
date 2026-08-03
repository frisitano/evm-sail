#ifndef EVMSAIL_OPTIMIZED_LIB_RLP_CODECS_TRANSACTIONS_H
#define EVMSAIL_OPTIMIZED_LIB_RLP_CODECS_TRANSACTIONS_H

#include "evmsail/prelude.h"

/* Decodes one complete transaction while retaining variable fields as input slices. */
struct TransactionFields decode_transaction(
    struct StatelessInputSliceFields transaction,
    struct StatelessInputSliceFields public_key, uint8_t blob_limit);

#endif
