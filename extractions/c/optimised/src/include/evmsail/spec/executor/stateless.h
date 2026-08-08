#pragma once

#include "evmsail/spec/executor/payload.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

void validate_executed_block(struct Block block, struct BlockExecutionResult result);

void verify_stateless_payload(struct StatelessInputRef input_ref);


#ifdef __cplusplus
}
#endif
