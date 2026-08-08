#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

uint8_t decode_single_stack_index(uint64_t immediate)
{
  bool valid = deep_stack_immediate_valid(immediate);
  if (!valid) {
    __builtin_trap();
  }
  uint8_t tmp_3_2928 = (uint8_t)immediate;
  if (tmp_3_2928 <= UINT8_C(90)) {
    return ((uint8_t)((uint32_t)UINT8_C(145) + (uint32_t)tmp_3_2928));
  }
  if (UINT8_C(128) > tmp_3_2928) {
    __builtin_trap();
  }
  return ((uint8_t)((uint32_t)tmp_3_2928 - (uint32_t)UINT8_C(111)));
}

struct tuple_uint_16_uint_16 decode_exchange_stack_indices(uint64_t immediate)
{
  bool valid = exchange_immediate_valid(immediate);
  if (!valid) {
    __builtin_trap();
  }
  uint64_t shifted = (immediate ^ UINT64_C(0x8F));
  uint8_t tmp_3_2923 = (uint8_t)(UINT64_C(0xF) & (shifted >> UINT8_C(4)));
  uint8_t tmp_3_2924 = (uint8_t)(UINT64_C(0xF) & (shifted >> UINT8_C(0)));
  if (tmp_3_2923 < tmp_3_2924) {
    return ((struct tuple_uint_16_uint_16){.tup0 = ((uint16_t)UINT8_C(1) + (uint16_t)tmp_3_2923), .tup1 = ((uint16_t)UINT8_C(1) + (uint16_t)tmp_3_2924)});
  }
  return ((struct tuple_uint_16_uint_16){.tup0 = ((uint16_t)UINT8_C(1) + (uint16_t)tmp_3_2924), .tup1 = ((uint16_t)UINT8_C(29) - (uint16_t)tmp_3_2923)});
}

