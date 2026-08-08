#pragma once

#include "evmsail/spec/lib/rlp/codecs/transaction_signing.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

uint8_t tx_signature_parity(uint64_t chain_id, enum TxSignatureScheme scheme, u256 v);

bool tx_auth_valid(bytes20 sender, bytes32 h, uint8_t parity, u256 r, u256 s);


#ifdef __cplusplus
}
#endif
