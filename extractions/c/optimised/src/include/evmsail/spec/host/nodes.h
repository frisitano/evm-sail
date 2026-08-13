#pragma once

#include "evmsail/spec/host/code.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

void nodedb_reset(void);

Bytes nodedb_lookup(bytes32 /* arg_0 */);


#ifdef __cplusplus
}
#endif
