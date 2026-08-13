#include "evmsail/spec.h"
#include "evmsail/spec/support.h"

u256 legacy_sig_chain_id(u256 v)
{
  u256 adjusted_v = !u256_lt_u64(v, UINT8_C(35)) ? u256_sub_u64(v, UINT8_C(35)) : u256_add_u64(u256_sub((u256){{UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615), UINT64_C(18446744073709551615)}}, u256_of_fbits(((uint8_t)((uint32_t)UINT8_C(35) - (uint32_t)(uint8_t)u256_to_u64_unchecked(v))))), UINT8_C(1));
  return word_div_word_u256_uint8_t_to_u256(adjusted_v, UINT8_C(2));
}

