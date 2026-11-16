#pragma once

#include "evmsail/spec/lib/htr.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

void commit_validation_result(bytes32 root, bool success, uint64_t chain_id, uint64_t schema_id);

void write_validation_result(struct StatelessInputRef input_ref, bool success);

void write_invalid_result(void);

void commit_validation_result_bytes32_bool_uint8_t_uint64_t_to_unit(bytes32 root, bool success,
                                                                    uint8_t chain_id,
                                                                    uint64_t schema_id);

extern const uint8_t RESULT_LENGTH;

#ifdef __cplusplus
}
#endif
