#ifndef EVMSAIL_OPTIMIZED_LIB_RLP_CODECS_TRANSACTION_SIGNING_H
#define EVMSAIL_OPTIMIZED_LIB_RLP_CODECS_TRANSACTION_SIGNING_H

#include "evmsail/prelude.h"

Hash32 tx_signing_hash(
    uint8_t envelope_type, struct zStatelessInputSliceFields content,
    U256 v);

#endif
