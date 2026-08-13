#pragma once

#include "evmsail/spec/host/stack.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

void frame_stack_push(struct FrameContinuation /* arg_0 */);


#ifdef __cplusplus
}
#endif
