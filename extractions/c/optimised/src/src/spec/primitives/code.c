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

void create_letbind_33(void) {

  Bytes let_value_3_44 = code_slice(EMPTY_CODE_REGION_SLICE);
  EMPTY_CODE_SLICE = let_value_3_44;
}
void kill_letbind_33(void) {
}

bool deep_stack_immediate_valid(uint64_t immediate)
{
  uint8_t tmp_3_3564 = (uint8_t)immediate;
  return (bool)((tmp_3_3564 <= UINT8_C(90)) || (UINT8_C(128) <= tmp_3_3564));
}

bool exchange_immediate_valid(uint64_t immediate)
{
  uint8_t tmp_3_3562 = (uint8_t)immediate;
  return (bool)((tmp_3_3562 <= UINT8_C(81)) || (UINT8_C(128) <= tmp_3_3562));
}

struct CodeFields analyzed_code(Bytes bytes, uint8_t * jumpdests)
{
  return ((struct CodeFields){.bytes = bytes.bytes, .jumpdests = jumpdests, .len = bytes.len});
}

Bytes code_bytes(struct CodeFields code)
{
  return ((Bytes){.bytes = code.bytes, .len = code.len});
}

void create_letbind_34(void) {

  struct CodeFields let_value_3_45 = analyzed_code(EMPTY_CODE_SLICE, EMPTY_JUMP_TABLE);
  EMPTY_CODE = let_value_3_45;
}
void kill_letbind_34(void) {
}

