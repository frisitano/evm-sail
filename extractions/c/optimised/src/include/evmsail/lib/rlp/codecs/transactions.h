#ifndef EVMSAIL_OPTIMIZED_LIB_RLP_CODECS_TRANSACTIONS_H
#define EVMSAIL_OPTIMIZED_LIB_RLP_CODECS_TRANSACTIONS_H

#include "evmsail/prelude.h"

#include <stdint.h>

struct TransactionFields decode_transaction(Bytes transaction, Bytes public_key,
                                            uint8_t blob_limit);
PreparedAuthorizationList prepare_authorizations(struct AuthorizationListRefFields authorizations);
struct Authorization prepared_authorization_head(PreparedAuthorizationList authorizations);
PreparedAuthorizationList prepared_authorization_tail(PreparedAuthorizationList authorizations,
                                                      uint16_t count);

#endif
