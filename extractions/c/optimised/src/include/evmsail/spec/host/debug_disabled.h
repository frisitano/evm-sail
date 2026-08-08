#pragma once

#include "evmsail/spec/lib/state_trie.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

void validation_debug_capture_block_gas(void);

void validation_debug_record(uint8_t _stage, enum FatalError _reason);


#ifdef __cplusplus
}
#endif
