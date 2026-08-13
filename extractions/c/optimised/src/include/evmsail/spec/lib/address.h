#pragma once

#include "evmsail/spec/lib/rlp/codecs/address.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

bytes20 create2_address_(bytes20 sender, u256 salt, bytes32 init_hash);


#ifdef __cplusplus
}
#endif
