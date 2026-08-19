#pragma once

#include "evmsail/spec/lib/htr.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

void write_prefix(bytes32 root, bool success);

void commit_validation_result(bytes32 root, bool success, Bytes chain_config);

void write_validation_result(struct StatelessInputRef input_ref, bool success);

void write_invalid_result(void);

extern const uint8_t RESULT_METADATA_LENGTH;



#ifdef __cplusplus
}
#endif
