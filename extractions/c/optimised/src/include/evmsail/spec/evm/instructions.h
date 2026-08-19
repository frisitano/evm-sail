#pragma once

#include "evmsail/spec/evm/precompiles.h"

#include "evmsail/spec/abi.h"
#include "evmsail/host/stack.h"
#include "evmsail/host/types.h"
#ifdef __cplusplus
extern "C" {
#endif

// struct tuple_uint_16_uint_16
struct tuple_uint_16_uint_16 {
  uint16_t tup0;
  uint16_t tup1;
};

uint8_t decode_single_stack_index(uint64_t immediate);

uint16_t decode_exchange_stack_indices(uint64_t immediate, uint16_t *restrict field_0_8_1370);


#ifdef __cplusplus
}
#endif
