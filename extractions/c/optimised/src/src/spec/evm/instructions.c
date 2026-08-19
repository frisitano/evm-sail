#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

uint8_t decode_single_stack_index(uint64_t immediate)
{
  if (!(deep_stack_immediate_valid(immediate))) {
    __builtin_trap();
  }
  if ((uint8_t)immediate <= UINT8_C(90)) {
    return ((uint8_t)((uint32_t)UINT8_C(145) + (uint32_t)(uint8_t)immediate));
  }
  if (UINT8_C(128) > (uint8_t)immediate) {
    __builtin_trap();
  }
  return ((uint8_t)((uint32_t)(uint8_t)immediate - (uint32_t)UINT8_C(111)));
}

uint16_t decode_exchange_stack_indices(uint64_t immediate, uint16_t *restrict field_0_8_1370)
{
  if (!(exchange_immediate_valid(immediate))) {
    __builtin_trap();
  }
  if ((uint8_t)(UINT64_C(0xF) & ((immediate ^ UINT64_C(0x8F)) >> UINT8_C(4))) < (uint8_t)(UINT64_C(0xF) & ((immediate ^ UINT64_C(0x8F)) >> UINT8_C(0)))) {
    (*field_0_8_1370) = ((uint16_t)UINT8_C(1) + (uint16_t)(uint8_t)(UINT64_C(0xF) & ((immediate ^ UINT64_C(0x8F)) >> UINT8_C(4))));
    return ((uint16_t)UINT8_C(1) + (uint16_t)(uint8_t)(UINT64_C(0xF) & ((immediate ^ UINT64_C(0x8F)) >> UINT8_C(0))));
  }
  (*field_0_8_1370) = ((uint16_t)UINT8_C(1) + (uint16_t)(uint8_t)(UINT64_C(0xF) & ((immediate ^ UINT64_C(0x8F)) >> UINT8_C(0))));
  return ((uint16_t)UINT8_C(29) - (uint16_t)(uint8_t)(UINT64_C(0xF) & ((immediate ^ UINT64_C(0x8F)) >> UINT8_C(4))));
}

