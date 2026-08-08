#pragma once

#include "evmsail/spec/host/state.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

void ancestor_hash_write(uint8_t /* arg_0 */, bytes32 /* arg_1 */);

bytes32 ancestor_hash_read(uint8_t /* arg_0 */);


#ifdef __cplusplus
}
#endif
