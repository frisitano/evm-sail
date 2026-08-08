#pragma once

#include "evmsail/spec/lib/rlp/decoding.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

bytes20 create_address(bytes20 sender, uint64_t nonce);


#ifdef __cplusplus
}
#endif
