#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

uint8_t * const EMPTY_JUMP_TABLE = INT64_C(0);


Bytes EMPTY_CODE_SLICE;


struct CodeFields EMPTY_CODE;


Bytes code_slice(Bytes bytes)
{
  return bytes;
}

Bytes validated_code_slice(Bytes bytes)
{
  if (bytes.len > UINT32_C(4294967263)) {
    __builtin_trap();
  }
  return code_slice(bytes);
}

void create_letbind_34(void) {
    EMPTY_CODE_SLICE = (code_slice(EMPTY_CODE_REGION_SLICE));
}
void kill_letbind_34(void) {
}

enum DeepStackOperation deep_stack_operation(uint8_t opcode)
{
  if (opcode == UINT8_C(230)) {
    return DeepStackDuplicate;
  }
  if (opcode == UINT8_C(231)) {
    return DeepStackSwap;
  }
  if (opcode == UINT8_C(232)) {
    return DeepStackExchange;
  }
  return NotDeepStackOperation;
}

bool deep_stack_immediate_valid(uint64_t immediate)
{
  return (bool)(((uint8_t)immediate <= UINT8_C(90)) || (UINT8_C(128) <= (uint8_t)immediate));
}

bool exchange_immediate_valid(uint64_t immediate)
{
  return (bool)(((uint8_t)immediate <= UINT8_C(81)) || (UINT8_C(128) <= (uint8_t)immediate));
}

bool deep_stack_operation_immediate_valid(enum DeepStackOperation operation, uint64_t immediate)
{
  switch (operation) {
  case DeepStackDuplicate:
  case DeepStackSwap:
    return deep_stack_immediate_valid(immediate);
  case DeepStackExchange:
    return exchange_immediate_valid(immediate);
  case NotDeepStackOperation:
    return false;
  }
}

struct CodeFields analyzed_code(Bytes bytes, uint8_t * jumpdests)
{
  return ((struct CodeFields){.bytes = bytes.bytes, .jumpdests = jumpdests, .len = bytes.len});
}

Bytes code_bytes(struct CodeFields code)
{
  return ((Bytes){.bytes = code.bytes, .len = code.len});
}

void create_letbind_35(void) {
    EMPTY_CODE = (analyzed_code(EMPTY_CODE_SLICE, EMPTY_JUMP_TABLE));
}
void kill_letbind_35(void) {
}

