#pragma once

#include "evmsail/spec/executor/result.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

void fatal_error_set_input(struct StatelessInputRef _input_ref);

void zmain(void);

void evmsail_model_init(void);


#ifdef __cplusplus
}
#endif
