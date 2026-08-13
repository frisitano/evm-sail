#pragma once

#include "evmsail/spec/kernel/environment.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

enum PrecompileId precompile_id_for_address(bytes20 bytes);

bool k_account_is_warm(bytes20 a);

void k_account_mark_warm(bytes20 a);

void k_prewarm_slot(bytes20 a, u256 s);

struct Account k_aload_(bytes20 a);

u256 k_tload(bytes20 a, u256 s);

void k_tstore(bytes20 a, u256 s, u256 v);


#ifdef __cplusplus
}
#endif
