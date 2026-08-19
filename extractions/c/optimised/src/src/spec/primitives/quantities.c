#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

struct MemoryRangeFields EMPTY_MEMORY_RANGE;


struct MemoryAccessFields EMPTY_MEMORY_ACCESS;


bool neq_anything_EPrecompileId__(enum PrecompileId x, enum PrecompileId y)
{
  bool eq_anything_result_2_2757 = eq_PrecompileId(y, x);
  return (bool)(!eq_anything_result_2_2757);
}

uint64_t word_of_account_nonce(uint64_t value)
{
  return value;
}

uint64_t word_of_withdrawal_amount(uint64_t value)
{
  return value;
}

uint64_t word_of_slot_number(uint64_t value)
{
  return value;
}

uint64_t word_of_block_number(uint64_t value)
{
  return value;
}

uint64_t word_of_block_timestamp(uint64_t value)
{
  return value;
}

uint64_t word_of_chain_identifier(uint64_t value)
{
  return value;
}

struct MemoryRangeFields memory_range(uint32_t off, uint32_t len)
{
  return ((struct MemoryRangeFields){.len = len, .off = off});
}

void create_letbind_7(void) {

  struct MemoryRangeFields let_value_3_10;
  struct MemoryRangeFields tmp_3_9;
  tmp_3_9.len = UINT32_C(0);
  tmp_3_9.off = UINT32_C(0);
  let_value_3_10 = tmp_3_9;
  EMPTY_MEMORY_RANGE = let_value_3_10;
}
void kill_letbind_7(void) {
}

void create_letbind_8(void) {

  struct MemoryAccessFields let_value_3_12;
  struct MemoryAccessFields tmp_3_11;
  tmp_3_11.range = EMPTY_MEMORY_RANGE;
  tmp_3_11.requested_height = UINT32_C(0);
  let_value_3_12 = tmp_3_11;
  EMPTY_MEMORY_ACCESS = let_value_3_12;
}
void kill_letbind_8(void) {
}

u256 word_of_source_byte_count(uint32_t value)
{
  return word_of_nat_byte_count_uint32_t_to_u256(value);
}

struct MemoryRangeFields memory_range_uint8_t_uint8_t_to_struct_MemoryRangeFields(uint8_t off, uint8_t len)
{
  return ((struct MemoryRangeFields){.len = (uint32_t)len, .off = (uint32_t)off});
}

u256 word_of_nat_byte_count_u128_to_u256(u128 value)
{
  return u256_u128_to_u256(value);
}

u256 word_of_nat_byte_count_uint16_t_to_u256(uint16_t value)
{
  return u256_uint16_t_to_u256(value);
}

u256 word_of_nat_byte_count_uint32_t_to_u256(uint32_t value)
{
  return u256_uint32_t_to_u256(value);
}

u256 word_of_nat_byte_count_uint64_t_to_u256(uint64_t value)
{
  return u256_uint64_t_to_u256(value);
}

u256 word_of_nat_byte_count_uint8_t_to_u256(uint8_t value)
{
  return u256_uint8_t_to_u256(value);
}

