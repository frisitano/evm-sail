#pragma once

#include "evmsail/spec/lib/tx.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct AccessListDecode
struct AccessListDecode {
  uint32_t address_count;
  uint32_t slot_count;
};

PreparedAuthorizationList prepare_authorizations_(struct AuthorizationListRefFields authorizations);

struct Authorization prepared_authorization_head_(PreparedAuthorizationList authorizations);

PreparedAuthorizationList prepared_authorization_tail_(PreparedAuthorizationList authorizations, uint16_t count);

struct TransactionFields rlp_decode_tx(Bytes tx, Bytes pubkey, uint8_t blob_limit);


#ifdef __cplusplus
}
#endif
