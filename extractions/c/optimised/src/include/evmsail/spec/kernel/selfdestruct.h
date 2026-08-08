#pragma once

#include "evmsail/spec/kernel/code.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

void k_selfdestruct(bytes20 a);

bool k_is_selfdestructed(bytes20 a);

void k_mark_created(bytes20 a);

bool k_was_created(bytes20 a);

void k_zero_balance(bytes20 a);


#ifdef __cplusplus
}
#endif
