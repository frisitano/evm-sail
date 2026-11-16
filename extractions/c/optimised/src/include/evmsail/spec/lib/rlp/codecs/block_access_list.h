#pragma once

#include "evmsail/spec/executor/system_calls.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

Bytes bal_ref_cursor(struct RlpFieldRef f);

u256 bal_ref_word(struct RlpFieldRef f);

Bytes bal_ref_bytes(struct RlpFieldRef f);

uint64_t bal_ref_uint64(struct RlpFieldRef f);

void bal_expect_end(Bytes cursor);


#ifdef __cplusplus
}
#endif
